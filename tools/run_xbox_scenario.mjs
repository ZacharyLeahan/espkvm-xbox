// SPDX-License-Identifier: Apache-2.0
// Inspired by OXOS bootrun.py's repeat/classify/evidence workflow; no code copied.
import {readFile, mkdir, writeFile, appendFile} from 'node:fs/promises';
import {join, resolve} from 'node:path';
import {pathToFileURL} from 'node:url';
import {parseArgs} from 'node:util';
import {setTimeout as sleep} from 'node:timers/promises';
import {decodeDiagnostic, diagnosticChecks} from './xbox_diagnostic.mjs';

const digital = ['up', 'down', 'left', 'right', 'start', 'back', 'ls', 'rs'];
const analog = ['a', 'b', 'x', 'y', 'black', 'white', 'lt', 'rt'];
const fields = ['signal', 'width', 'height', 'interlaced', 'inputHz', 'fps',
  'modeChanges', 'viewers', 'codec', 'frames', 'frameAgeMs'];
function integer(n, min, max) { return Number.isInteger(n) && n >= min && n <= max; }
function keys(object, allowed) {
  if (!object || typeof object !== 'object' || Array.isArray(object) ||
      Object.keys(object).some(k => !allowed.includes(k))) throw new Error('Unknown or invalid scenario fields');
}
export function validate(plan) {
  keys(plan, ['name', 'start_state', 'steps']);
  if (typeof plan.name !== 'string' || !plan.name.trim() ||
      typeof plan.start_state !== 'string' || !plan.start_state.trim() ||
      !Array.isArray(plan.steps) || !integer(plan.steps.length, 1, 200)) throw new Error('Scenario needs name, start_state and 1..200 steps');
  for (const step of plan.steps) {
    keys(step, ['name', 'buttons', 'player', 'hold_ms', 'wait_ms', 'expect', 'screenshot', 'diagnostic']);
    if (typeof step.name !== 'string' || !step.name.trim()) throw new Error('Each step needs a name');
    if (step.buttons !== undefined) {
      if (!Array.isArray(step.buttons) || !integer(step.buttons.length, 1, 16) ||
          new Set(step.buttons).size !== step.buttons.length ||
          step.buttons.some(b => ![...digital, ...analog].includes(b)) ||
          !integer(step.hold_ms, 50, 2000) || !integer(step.player ?? 1, 1, 4)) throw new Error('Invalid buttons/player/hold_ms');
    } else if (step.hold_ms !== undefined || step.player !== undefined) throw new Error('hold_ms/player require buttons');
    if (step.wait_ms !== undefined && !integer(step.wait_ms, 0, 120000)) throw new Error('Invalid wait_ms');
    if (step.screenshot !== undefined && typeof step.screenshot !== 'boolean') throw new Error('Invalid screenshot');
    if (step.diagnostic !== undefined && step.diagnostic !== true) throw new Error('diagnostic must be true when specified');
    if (step.expect !== undefined) {
      keys(step.expect, ['signal', 'width', 'height']);
      if (!Object.keys(step.expect).length || Object.entries(step.expect).some(([k, v]) =>
        k === 'signal' ? typeof v !== 'boolean' : !integer(v, 1, 8192))) throw new Error('Invalid expected video fields');
    }
    if (!step.buttons && step.wait_ms === undefined && !step.expect && !step.screenshot && !step.diagnostic) throw new Error('Empty step');
  }
  return plan;
}
export function report(buttons) {
  const r = new Uint8Array(20); r[1] = 20;
  for (const b of buttons) {
    if (digital.includes(b)) r[2] |= 1 << digital.indexOf(b);
    else r[4 + analog.indexOf(b)] = 255;
  }
  return r;
}

export async function run({origin, plan, output, runs = 1, timeoutMs = 120000,
  buildId, kvmBuildId}) {
  const controller = new AbortController();
  const {signal} = controller;
  const started = performance.now();
  const results = [];
  let ws, owned = false, refresh, runNumber = 0, stopped = false;
  await mkdir(output, {mode: 0o700}); // Existing evidence must never be overwritten.
  const emit = (event, data = {}) => appendFile(join(output, 'events.jsonl'), JSON.stringify({
    schema: 1, event, utc: new Date().toISOString(), elapsed_ms: Math.round(performance.now() - started),
    run: runNumber, ...data}) + '\n');
  const fail = reason => { if (!signal.aborted) controller.abort(new Error(reason)); };
  const interrupt = () => fail('Operator interrupted');
  process.once('SIGINT', interrupt); process.once('SIGTERM', interrupt);
  const deadline = setTimeout(() => fail('Session deadline exceeded'), timeoutMs);
  // Release immediately on abort, rather than waiting for HTTP/file operations.
  const release = () => {
    clearInterval(refresh);
    if (owned && ws?.readyState === WebSocket.OPEN) ws.send(new Uint8Array([5]));
  };
  signal.addEventListener('abort', release, {once: true});
  const send = bytes => {
    signal.throwIfAborted();
    if (!owned || ws.readyState !== WebSocket.OPEN) throw new Error('Controller session unavailable');
    ws.send(bytes);
  };
  const get = async (path, type, limit, requestSignal = signal) => {
    const response = await fetch(new URL(path, origin), {redirect: 'error',
      signal: AbortSignal.any([signal, requestSignal, AbortSignal.timeout(5000)])});
    if (!response.ok || response.headers.get('content-type')?.split(';')[0] !== type) {
      await response.body?.cancel();
      throw new Error(`Evidence endpoint failed: ${path} (HTTP ${response.status})`);
    }
    const chunks = []; let length = 0;
    for await (const chunk of response.body) {
      length += chunk.length;
      if (length > limit) throw new Error('Evidence response exceeds limit');
      chunks.push(chunk);
    }
    return Buffer.concat(chunks);
  };
  let panelNumber = 0;
  const panel = async (index, phase, phaseSignal) => {
    const jpeg = await get('/api/v1/video/frame.jpg', 'image/jpeg', 4 * 1024 * 1024, phaseSignal);
    const file = `run-${runNumber}-step-${index}-panel-${panelNumber++}.jpg`;
    await writeFile(join(output, file), jpeg, {flag: 'wx'});
    const observation = await decodeDiagnostic(jpeg, phaseSignal);
    await emit('diagnostic_panel', {index, phase, file, observation});
    return observation;
  };
  const waitPanel = async (index, phase, predicate) => {
    const until = performance.now() + 5000;
    const phaseSignal = AbortSignal.any([signal, AbortSignal.timeout(5000)]);
    let latest, lastError;
    do {
      signal.throwIfAborted();
      try {
        latest = await panel(index, phase, phaseSignal);
        if (predicate(latest)) return latest;
      } catch (error) {
        lastError = error.message;
        await emit('diagnostic_read_failed', {index, phase, reason: lastError});
      }
      if (phaseSignal.aborted) break;
      await sleep(200, undefined, {signal});
    } while (performance.now() < until);
    if (!latest) throw new Error(`Cannot read diagnostic panel: ${lastError}`);
    return latest;
  };
  let final;
  try {
    await writeFile(join(output, 'plan.json'), JSON.stringify(plan, null, 2));
    await emit('session', {scenario: plan.name, start_state: plan.start_state,
      build_id: buildId, kvm_build_id: kvmBuildId, runs, timeout_ms: timeoutMs,
      note: 'No power control, deployment or automatic reconnect. Only XKD1 diagnostic panels have automatic application assertions.'});
    signal.throwIfAborted();
    const url = new URL('/ws', origin); url.protocol = url.protocol === 'https:' ? 'wss:' : 'ws:';
    ws = new WebSocket(url); ws.binaryType = 'arraybuffer';
    await new Promise((accept, reject) => {
      const timer = setTimeout(() => fail('Controller takeover timed out'), 5000);
      const onAbort = () => { clearTimeout(timer); reject(signal.reason); };
      signal.addEventListener('abort', onAbort, {once: true});
      if (signal.aborted) { onAbort(); return; }
      ws.onopen = () => { if (!signal.aborted) ws.send(new Uint8Array([7])); };
      ws.onerror = () => fail('Controller WebSocket error');
      ws.onclose = () => { if (!stopped) fail('Controller disconnected; manual recovery required'); };
      ws.onmessage = ({data}) => {
        const bytes = new Uint8Array(data);
        if (bytes[0] !== 0x83 || bytes.length < 2) return;
        if (bytes[1] === 1 && !owned) {
          owned = true; clearTimeout(timer); signal.removeEventListener('abort', onAbort); accept();
        } else if (bytes[1] !== 1 && owned) {
          owned = false; fail('Controller ownership lost; no takeover retry');
        }
      };
    });
    signal.throwIfAborted(); release();
    for (runNumber = 1; runNumber <= runs; runNumber++) {
      await emit('run_started');
      const checks = [];
      for (let index = 0; index < plan.steps.length; index++) {
        signal.throwIfAborted();
        const step = plan.steps[index];
        await emit('step_started', {index, step});
        let baseline;
        if (step.diagnostic) {
          const first = await waitPanel(index, 'baseline-first', () => true);
          baseline = await waitPanel(index, 'baseline-fresh', value =>
            value.frames > first.frames && value.elapsed_ms > first.elapsed_ms && value.pattern_ok);
          if (!baseline.pattern_ok || baseline.frames <= first.frames || baseline.elapsed_ms <= first.elapsed_ms ||
              baseline.players.some(p => p.held !== 0) || (step.buttons && !baseline.players[(step.player ?? 1) - 1].present)) {
            throw new Error('Diagnostic baseline is stale, wrong, non-neutral or missing target player; no input sent');
          }
        }
        if (step.buttons) {
          const packet = new Uint8Array([8, step.player ?? 1, ...report(step.buttons)]);
          send(packet);
          refresh = setInterval(() => { if (!signal.aborted && owned && ws.readyState === WebSocket.OPEN) ws.send(packet); }, 100);
          try { await sleep(step.hold_ms, undefined, {signal}); }
          finally { release(); }
          await emit('input_released', {index});
        }
        if (step.wait_ms) await sleep(step.wait_ms, undefined, {signal});
        if (baseline) {
          const after = await waitPanel(index, 'after', value =>
            diagnosticChecks(baseline, value, step.buttons, step.player).every(c => c.passed));
          for (const check of diagnosticChecks(baseline, after, step.buttons, step.player)) {
            checks.push({index, ...check}); await emit('check', {index, ...check});
          }
          if (checks.some(c => !c.passed)) {
            const result = {run: runNumber, outcome: 'FAILED_CHECK', checks};
            results.push(result); await emit('run_finished', result);
            throw new Error('Diagnostic progress, input acknowledgment/release or reference-color check failed');
          }
        }
        if (step.expect || step.screenshot) {
          const status = JSON.parse((await get('/api/v1/video/status', 'application/json', 65536)).toString());
          if (!status || typeof status.signal !== 'boolean') throw new Error('Malformed video status');
          const video = Object.fromEntries(fields.filter(k => k in status).map(k => [k, status[k]]));
          await emit('video', {index, video});
          if (step.screenshot) {
            const jpeg = await get('/api/v1/video/frame.jpg', 'image/jpeg', 4 * 1024 * 1024);
            if (jpeg[0] !== 255 || jpeg[1] !== 216 || jpeg.at(-2) !== 255 || jpeg.at(-1) !== 217) throw new Error('Incomplete JPEG');
            const file = `run-${runNumber}-step-${index}.jpg`;
            await writeFile(join(output, file), jpeg, {flag: 'wx'});
            await emit('screenshot', {index, file, review: 'required'});
          }
          for (const [field, expected] of Object.entries(step.expect ?? {})) {
            const check = {index, field, expected, actual: video[field] ?? null, passed: video[field] === expected};
            checks.push(check); await emit('check', check);
          }
          if (checks.some(c => !c.passed)) {
            const result = {run: runNumber, outcome: 'FAILED_CHECK', checks};
            results.push(result); await emit('run_finished', result);
            throw new Error('Video expectation failed; stop for review/manual recovery');
          }
        }
        await emit('step_finished', {index});
      }
      const completeDiagnostic = plan.steps.every(step => step.diagnostic);
      const result = {run: runNumber, outcome: completeDiagnostic ? 'PASS_DIAGNOSTIC' : 'INCONCLUSIVE', checks,
        reason: completeDiagnostic ? 'XKD1 progress, reference colors and requested input acknowledgments passed; not a game qualification.' :
          'Sequence completed; Xbox screen/application behavior requires review.'};
      results.push(result); await emit('run_finished', result);
    }
    signal.throwIfAborted();
    final = {outcome: results.every(r => r.outcome === 'PASS_DIAGNOSTIC') ? 'PASS_DIAGNOSTIC' : 'INCONCLUSIVE',
      results, reason: 'Diagnostic PASS covers only the diagnostic title checks; review game evidence separately.'};
  } catch (error) {
    const reason = signal.aborted ? signal.reason.message : error.message;
    if (!results.some(r => r.run === runNumber)) results.push({run: runNumber, outcome: 'NEEDS_RECOVERY', reason});
    await emit('stopped', {reason});
    final = {outcome: results.at(-1)?.outcome ?? 'NEEDS_RECOVERY', reason, results};
  } finally {
    clearTimeout(deadline); release(); stopped = true;
    ws?.close();
    if (!signal.aborted) controller.abort();
    process.removeListener('SIGINT', interrupt); process.removeListener('SIGTERM', interrupt);
  }
  await writeFile(join(output, 'summary.json'), JSON.stringify(final, null, 2));
  return final;
}

async function main() {
  const {values} = parseArgs({options: {
    scenario: {type: 'string'}, url: {type: 'string'}, output: {type: 'string'},
    runs: {type: 'string', default: '1'}, 'timeout-seconds': {type: 'string', default: '120'},
    'build-id': {type: 'string'}, 'kvm-build-id': {type: 'string'}, execute: {type: 'boolean', default: false},
  }});
  if (!values.scenario) throw new Error('Use --scenario FILE [--execute --url ORIGIN --output NEW_DIR --build-id ID --kvm-build-id ID] [--runs 1..100 --timeout-seconds 1..3600]');
  const plan = validate(JSON.parse(await readFile(values.scenario, 'utf8')));
  const runs = Number(values.runs), timeoutMs = Number(values['timeout-seconds']) * 1000;
  if (!integer(runs, 1, 100) || !integer(timeoutMs, 1000, 3600000)) throw new Error('Invalid runs or timeout');
  if (!values.execute) { console.log(JSON.stringify({mode: 'DRY_RUN', runs, timeout_ms: timeoutMs, plan}, null, 2)); return; }
  const url = new URL(values.url);
  if (!['http:', 'https:'].includes(url.protocol) || url.username || url.password || url.search || url.hash || url.pathname !== '/') throw new Error('Use an HTTP(S) origin without credentials/path');
  if (!values.output || !values['build-id'] || !values['kvm-build-id']) throw new Error('Execution requires output and both build IDs');
  const result = await run({origin: url.origin, plan, output: values.output, runs, timeoutMs,
    buildId: values['build-id'], kvmBuildId: values['kvm-build-id']});
  console.log(JSON.stringify(result));
  process.exitCode = result.outcome === 'PASS_DIAGNOSTIC' ? 0 : result.outcome === 'INCONCLUSIVE' ? 2 : 1;
}
if (process.argv[1] && pathToFileURL(resolve(process.argv[1])).href === import.meta.url) {
  main().catch(error => { console.error(error.message); process.exitCode = 1; });
}
