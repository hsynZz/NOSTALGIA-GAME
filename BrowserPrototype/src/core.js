// Browser port of Source/Nostalgia/{Public,Private}/Core/NostalgiaCore.
// Keep numeric IDs and director semantics aligned; native parity tests exercise the production C++.
export const Stage = Object.freeze({Arrival:0, PhotoFound:1, DiscrepancySeen:2, BirthdayRestored:3, RoadUnlocked:4, DarkLampFound:5, Complete:6});
export const District = Object.freeze({Bedroom:0, Street:1, Garden:2, Interior:3, Finale:4});
export const EventId = Object.freeze({GardenLaughter:0, HomecomerSighting:1, WindowKnock:2, Count:3});
const NONE_TIME = -1000000;
const copy = value => JSON.parse(JSON.stringify(value));
const integer = (value,min,max) => Number.isInteger(value) && value >= min && value <= max;
const time = (value,now) => Number.isFinite(value) && value >= NONE_TIME && value <= now;

export function createState() {
  return {stage:0, minute:900, elapsed:0, randomState:2004, loops:0, openDoors:0,
    lastTransition:NONE_TIME, lastShock:NONE_TIME,
    history:Array.from({length:3},()=>({count:0,lastTime:NONE_TIME}))};
}
export function validateState(state) {
  if (!state || !integer(state.stage,0,6) || !Number.isFinite(state.minute) || state.minute < 0 || state.minute >= 1440 ||
    !Number.isFinite(state.elapsed) || state.elapsed < 0 || !integer(state.randomState,1,0xffffffff) ||
    !integer(state.loops,0,1000000) || !integer(state.openDoors,0,3) ||
    !time(state.lastTransition,state.elapsed) || !time(state.lastShock,state.elapsed) ||
    !Array.isArray(state.history) || state.history.length !== 3) return false;
  if (state.stage >= Stage.RoadUnlocked && state.loops === 0) return false;
  return state.history.every(item => item && integer(item.count,0,1000000) && time(item.lastTime,state.elapsed) &&
    (item.count === 0 ? item.lastTime === NONE_TIME : item.lastTime >= 0));
}
export function isNight(state) { return state.minute >= 1200 || state.minute < 360; }
export function advance(state,seconds) {
  if (!Number.isFinite(seconds) || seconds <= 0 || seconds > 86400) return;
  state.elapsed += seconds;
  state.minute = Math.min(1439,state.minute+seconds);
}
export function apply(state,action) {
  let next = state.stage;
  if (action === 'photo' && state.stage === 0) next = 1;
  if (action === 'compare' && state.stage === 1) next = 2;
  if (action === 'birthday' && state.stage === 2) next = 3;
  if (action === 'lamp' && state.stage === 4 && isNight(state)) next = 5;
  if (action === 'exit' && state.stage === 5) next = 6;
  if (next === state.stage) return false;
  state.stage = next;
  if (next === 3) state.minute = Math.max(state.minute,1204);
  return true;
}
export function traverseLoop(state) {
  if (state.elapsed-state.lastTransition < 3 || state.loops === 1000000) return false;
  state.lastTransition = state.elapsed;
  ++state.loops;
  if (state.stage === 3) state.stage = 4;
  return true;
}
const inside = (x,z,a,b,c,d,margin=0) => x >= a+margin && x <= b-margin && z >= c+margin && z <= d-margin;
export function districtAt(x,z) {
  if (!Number.isFinite(x) || !Number.isFinite(z)) return District.Bedroom;
  if (inside(x,z,-21,1,27,37)) return District.Interior;
  if (inside(x,z,-18,6,-4.2,4.2)) return District.Bedroom;
  if (inside(x,z,65,95,-7,7) || inside(x,z,69,91,28,32)) return District.Finale;
  if (x > 15.5 && x < 28.5 && z > 5 && z < 16.5) return District.Garden;
  return District.Street;
}
export function defaultRules() {
  return [
    {id:0,district:2,minimumStage:0,maximumStage:3,phase:0,minimumDwell:1,maximumSpeed:700,cooldown:120,maximumPlays:1,priority:100,scare:false,critical:true},
    {id:1,district:1,minimumStage:3,maximumStage:5,phase:0,minimumDwell:2,maximumSpeed:700,cooldown:90,maximumPlays:1,priority:90,scare:true,critical:true},
    {id:2,district:3,minimumStage:1,maximumStage:4,phase:2,minimumDwell:4,maximumSpeed:200,cooldown:100,maximumPlays:1,priority:20,scare:true,critical:false}
  ];
}
function validRule(rule) {
  return rule && integer(rule.id,0,2) && integer(rule.district,0,4) && integer(rule.minimumStage,0,6) &&
    integer(rule.maximumStage,rule.minimumStage,6) && integer(rule.phase,0,2) &&
    Number.isFinite(rule.minimumDwell) && rule.minimumDwell >= 0 && Number.isFinite(rule.maximumSpeed) && rule.maximumSpeed >= 0 &&
    Number.isFinite(rule.cooldown) && rule.cooldown >= 0 && integer(rule.maximumPlays,0,1000000) && Number.isInteger(rule.priority);
}
export function eligible(state,context,rule) {
  if (!validRule(rule) || state.stage === 6 || context.safeZone || context.threatened || context.district !== rule.district ||
    state.stage < rule.minimumStage || state.stage > rule.maximumStage || !Number.isFinite(context.dwell) || context.dwell < rule.minimumDwell ||
    !Number.isFinite(context.speed) || context.speed < 0 || context.speed > rule.maximumSpeed) return false;
  if ((rule.phase === 1 && isNight(state)) || (rule.phase === 2 && !isNight(state))) return false;
  if (context.available && !context.available[rule.id]) return false;
  const history = state.history[rule.id];
  return history.count < rule.maximumPlays && state.elapsed-history.lastTime >= rule.cooldown &&
    (!rule.scare || state.elapsed-state.lastShock >= 45);
}
function random(state) {
  let value = state.randomState;
  value ^= value << 13;
  value ^= value >>> 17;
  value ^= value << 5;
  state.randomState = value >>> 0;
  return state.randomState;
}
export function planEvent(state,context,rules=defaultRules()) {
  if (!validateState(state)) return {id:3,before:null,after:null};
  const before = copy(state), after = copy(state);
  let selected = null, ties = 0;
  for (const rule of rules) {
    if (!eligible(after,context,rule)) continue;
    const higher = !selected || (rule.critical && !selected.critical) ||
      (rule.critical === selected.critical && rule.priority > selected.priority);
    if (higher) { selected=rule; ties=1; }
    else if (rule.critical === selected.critical && rule.priority === selected.priority) {
      ++ties;
      if (random(after)%ties === 0) selected=rule;
    }
  }
  if (!selected) return {id:3,before,after};
  ++after.history[selected.id].count;
  after.history[selected.id].lastTime=after.elapsed;
  if (selected.scare) after.lastShock=after.elapsed;
  return {id:selected.id,before,after};
}
export function commitEvent(state,plan,delivered) {
  if (!delivered || !plan || plan.id === 3 || !plan.before || !validateState(plan.after) ||
    JSON.stringify(state) !== JSON.stringify(plan.before)) return false;
  Object.assign(state,copy(plan.after));
  return true;
}

export function canRestoreAt(x,z,stage) {
  if (!Number.isFinite(x) || !Number.isFinite(z) || !integer(stage,0,6)) return false;
  if (Math.abs(x-57) <= 1.34 && Math.abs(z) <= 5.04) return false;
  if (inside(x,z,-19,61,-23.5,23.5,.34) || inside(x,z,-21,1,27,37,.34)) return true;
  if (inside(x,z,65,95,-7,7,.34)) return stage >= 4;
  if (inside(x,z,69,91,28,32,.34)) return stage >= 5;
  return false;
}
function validSnapshot(state,player) {
  return validateState(state) && player && canRestoreAt(player.x,player.z,state.stage) &&
    Number.isFinite(player.yaw) && Math.abs(player.yaw) <= 1e6 && Number.isFinite(player.pitch) && Math.abs(player.pitch) <= Math.PI/2+.01 &&
    (state.stage < 3 || state.minute >= 1204) &&
    (state.loops === 0 ? state.lastTransition === NONE_TIME : state.lastTransition >= 0);
}
function sanitizedState(state) {
  const clean=createState();
  for (const name of Object.keys(clean)) clean[name]=copy(state[name]);
  return clean;
}
export function serializeSnapshot(state,player) {
  if (!validSnapshot(state,player)) throw new Error('Speichern hier nicht möglich: bitte aus Tür-/Übergangsbereichen treten.');
  return JSON.stringify({version:1,chapter:'EndlessSummer2004-Browser',state:sanitizedState(state),
    player:{x:player.x,z:player.z,yaw:player.yaw,pitch:player.pitch}});
}
export function parseSnapshot(text) {
  try {
    if (typeof text !== 'string' || text.length > 100000) return null;
    const data=JSON.parse(text);
    if (!data || data.version !== 1 || data.chapter !== 'EndlessSummer2004-Browser' || !validSnapshot(data.state,data.player)) return null;
    return {state:sanitizedState(data.state),player:{x:data.player.x,z:data.player.z,yaw:data.player.yaw,pitch:data.player.pitch}};
  } catch { return null; }
}

const objectives=[
  'Finde das Geburtstagsfoto auf deinem Schreibtisch.',
  'Vergleiche das Foto mit dem leeren Grundstück im Geburtstagsgarten.',
  'Ordne am Geburtstagstisch die Erinnerung neu.',
  'Folge der Straße noch einmal bis zum STOP-Schild.',
  'Untersuche die einzige Straßenlaterne, die dunkel bleibt.',
  'Betritt das Spielhaus und folge dem viel zu langen Flur.',
  'Du hast den Weg gefunden. Endless Summer – Testabschnitt abgeschlossen.'
];
const hints=[
  'Das Foto liegt auf dem Schreibtisch. Ansehen und E drücken. Türen öffnest du ebenfalls mit E.',
  'Verlasse das Haus. Der Geburtstagsgarten liegt rechts der Straße. Suche dort die Foto-Markierung.',
  'Auf dem Foto stehen vier Stühle am Tisch. Benutze den Geburtstagstisch mit E.',
  'Gehe auf der Straße in Richtung STOP. Die Erinnerung hat diesen Weg verändert.',
  'Am neuen Straßenende bleibt eine Laterne aus. Schaue sie an und drücke E.',
  'Neben der dunklen Laterne steht das Spielhaus. E am Eingang; am Ende des Flurs wartet die letzte Tür.',
  'Dies ist das Ende des Browser-Testabschnitts. Ein neues Spiel startet wieder im Kinderzimmer.'
];
export const objective = stage => objectives[stage] ?? 'Unbekannter Fortschritt';
export const hint = stage => hints[stage] ?? '';
