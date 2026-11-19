// SPDX-License-Identifier: Apache-2.0
import assert from 'node:assert/strict';
import {createRequire} from 'node:module';
import {createServer} from 'node:http';
import {once} from 'node:events';
import {mkdtemp, readFile, rm} from 'node:fs/promises';
import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {run, validate} from './run_xbox_scenario.mjs';
import {diagnosticChecks} from './xbox_diagnostic.mjs';
const require = createRequire(new URL('../web/package.json', import.meta.url));
const {WebSocketServer} = require('ws');
const fixture = fileURLToPath(new URL('./test_xbox_diagnostic.py', import.meta.url));
const pads = () => Array.from({length: 4}, () => ({present: true, held: 0, last: 0, presses: 0, releases: 0}));
const state = {frames: 1, elapsed_ms: 16, pattern_ok: true, players: pads()};
assert.equal(diagnosticChecks(state, state).every(c => c.passed), false, 'static panel cannot pass');
assert.throws(() => validate({name: 'bad', start_state: 'fixture', steps: [{name: 'bad', diagnostic: false}]}));

const root = await mkdtemp(join(tmpdir(), 'xbox-diagnostic-'));
try {
  for (const mode of ['complete', 'wrong-player', 'stuck-input', 'frozen-baseline', 'wrong-screen']) {
    let frames = 100, sent = 0;
    const players = pads();
    const server = createServer((req, res) => {
      if (req.url !== '/api/v1/video/frame.jpg') { res.writeHead(404); res.end(); return; }
      if (mode !== 'frozen-baseline') frames++;
      const generated = spawnSync('python3', [fixture, '--render-fixture'], {input: JSON.stringify({
        frames, elapsed_ms: frames * 16, players,
      })});
      assert.equal(generated.status, 0, generated.stderr.toString());
      res.writeHead(200, {'content-type': 'image/jpeg'});
      res.end(mode === 'wrong-screen' ? Buffer.from([255, 216, 255, 217]) : generated.stdout);
    });
    const wss = new WebSocketServer({server});
    wss.on('connection', socket => socket.on('message', bytes => {
      if (bytes[0] === 7) socket.send(Buffer.from([0x83, 1]));
      if (bytes[0] === 8) {
        sent++;
        const slot = mode === 'wrong-player' ? (bytes[1] % 4) : bytes[1] - 1;
        const p = players[slot];
        const mask = bytes[4] | Array.from(bytes.slice(6, 14)).reduce((v, b, bit) => v | (b ? 1 << (8 + bit) : 0), 0);
        if (mask && mask !== p.held) { p.last = mask; p.presses++; }
        p.held = mask;
      }
      if (bytes[0] === 5 && mode !== 'stuck-input') for (const p of players) {
        if (p.held) p.releases++;
        p.held = 0;
      }
    }));
    server.listen(0, '127.0.0.1'); await once(server, 'listening');
    const output = join(root, mode);
    try {
      const plan = validate({name: 'diagnostic', start_state: 'mock XKD1 title', steps: [
        {name: 'heartbeat', diagnostic: true},
        {name: 'player-2-a', buttons: ['a'], player: 2, hold_ms: 200, diagnostic: true},
      ]});
      const result = await run({origin: `http://127.0.0.1:${server.address().port}`, plan,
        output, runs: 2, timeoutMs: 20000, buildId: 'mock', kvmBuildId: 'mock'});
      assert.equal(result.outcome, mode === 'complete' ? 'PASS_DIAGNOSTIC' :
        ['wrong-player', 'stuck-input'].includes(mode) ? 'FAILED_CHECK' : 'NEEDS_RECOVERY', JSON.stringify(result));
      if (mode === 'complete') {
        assert.equal(result.results.length, 2);
        assert.ok(result.results.every(r => r.checks.every(c => c.passed)));
      }
      if (['frozen-baseline', 'wrong-screen'].includes(mode)) assert.equal(sent, 0, 'do not input into an unverified screen');
      const journal = await readFile(join(output, 'events.jsonl'), 'utf8');
      assert.ok(journal.includes(mode === 'wrong-screen' ? 'diagnostic_read_failed' : 'diagnostic_panel'));
      console.log(`PASS: diagnostic ${mode}`);
    } finally {
      for (const socket of wss.clients) socket.terminate();
      await new Promise(resolve => wss.close(resolve));
      await new Promise(resolve => server.close(resolve));
    }
  }
} finally { await rm(root, {recursive: true, force: true}); }
