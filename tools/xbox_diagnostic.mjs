// SPDX-License-Identifier: Apache-2.0
import {spawn} from 'node:child_process';
import {fileURLToPath} from 'node:url';
const names = ['up', 'down', 'left', 'right', 'start', 'back', 'ls', 'rs',
  'a', 'b', 'x', 'y', 'black', 'white', 'lt', 'rt'];
export function decodeDiagnostic(jpeg, signal) {
  return new Promise((accept, reject) => {
    const child = spawn('python3', [fileURLToPath(new URL('./decode_xbox_diagnostic.py', import.meta.url))],
      {stdio: ['pipe', 'pipe', 'pipe'], signal});
    let out = '', error = '';
    child.stdout.on('data', data => { out += data; });
    child.stderr.on('data', data => { error += data; });
    child.on('error', reject);
    child.stdin.on('error', () => {}); // Early decoder exit is reported by close.
    child.on('close', code => {
      if (code !== 0) reject(new Error(error.trim() || 'Diagnostic decoder failed (python3 and Pillow required)'));
      else { try { accept(JSON.parse(out)); } catch (e) { reject(e); } }
    });
    child.stdin.end(jpeg);
  });
}
export function diagnosticChecks(before, after, buttons, player = 1) {
  const advance = (a, b) => b > a; // Resets/wraps stop for review rather than implying progress.
  const checks = [
    {field: 'diagnostic.frames_advance', passed: advance(before.frames, after.frames)},
    {field: 'diagnostic.clock_advances', passed: advance(before.elapsed_ms, after.elapsed_ms)},
    {field: 'diagnostic.reference_colors', passed: before.pattern_ok && after.pattern_ok},
  ];
  if (buttons) {
    const expected = buttons.reduce((mask, b) => mask | (1 << names.indexOf(b)), 0);
    before.players.forEach((p, index) => {
      const q = after.players[index], target = index === player - 1;
      checks.push({field: `diagnostic.p${index + 1}.input`, expected: target ? expected : 0,
        actual: q.last, passed: target ? p.present && q.present && p.held === 0 &&
          q.held === 0 && q.last === expected && q.presses === p.presses + 1 &&
          q.releases === p.releases + 1 :
          q.present === p.present && q.held === 0 && q.presses === p.presses && q.releases === p.releases});
    });
  }
  return checks;
}
