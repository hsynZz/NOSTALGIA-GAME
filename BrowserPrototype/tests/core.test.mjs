import test from 'node:test';
import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {mkdtempSync, writeFileSync, rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {fileURLToPath} from 'node:url';
import path from 'node:path';
import {Stage,District,EventId,createState,advance,apply,traverseLoop,districtAt,defaultRules,
  validateState,planEvent,commitEvent,serializeSnapshot,parseSnapshot,canRestoreAt} from '../src/core.js';

const at=district=>({district,safeZone:false,threatened:false,dwell:10,speed:100,available:[true,true,true]});
const clone=value=>JSON.parse(JSON.stringify(value));
const birthday=()=>{const s=createState();apply(s,'photo');apply(s,'compare');apply(s,'birthday');return s;};

test('complete escape requires observation, memory, changed route and dark lamp',()=>{
  const s=createState();
  for(const a of ['exit','birthday','compare','lamp']) assert.equal(apply(s,a),false);
  for(const a of ['photo','compare','birthday']) assert.equal(apply(s,a),true);
  assert.equal(s.minute,1204);
  assert.equal(apply(s,'lamp'),false);
  assert.equal(traverseLoop(s),true);
  assert.equal(apply(s,'lamp'),true);
  assert.equal(apply(s,'exit'),true);
  assert.equal(s.stage,Stage.Complete);
  assert.equal(validateState(s),true);
});
test('clock, transition debounce and invalid values preserve boundaries',()=>{
  const s=createState();
  for(const delta of [-1,0,Infinity,NaN,86401])advance(s,delta);
  assert.equal(s.minute,900);
  advance(s,1.5);assert.equal(s.elapsed,1.5);
  assert.equal(traverseLoop(s),true);assert.equal(traverseLoop(s),false);
  advance(s,3);assert.equal(traverseLoop(s),true);
  advance(s,10000);assert.equal(s.minute,1439);
});
test('unavailable presentation, failed receipt and stale receipt never consume an event',()=>{
  const s=birthday(), c=at(District.Street);
  c.available[1]=false; assert.equal(planEvent(s,c).id,EventId.Count);
  c.available[1]=true; const before=clone(s), p=planEvent(s,c);
  assert.equal(p.id,EventId.HomecomerSighting);assert.deepEqual(s,before);
  assert.equal(commitEvent(s,p,false),false);assert.deepEqual(s,before);
  advance(s,.5);assert.equal(commitEvent(s,p,true),false);
  assert.equal(commitEvent(s,planEvent(s,c),true),true);
  assert.equal(s.history[1].count,1);assert.equal(planEvent(s,c).id,EventId.Count);
});
test('safe rooms, threat, phase and shared scare spacing suppress stacked events',()=>{
  const s=birthday(), c=at(District.Street);
  c.safeZone=true;assert.equal(planEvent(s,c).id,3);
  c.safeZone=false;c.threatened=true;assert.equal(planEvent(s,c).id,3);
  c.threatened=false;assert.equal(commitEvent(s,planEvent(s,c),true),true);
  const interior=at(District.Interior);
  assert.equal(planEvent(s,interior).id,3);
  advance(s,45);assert.equal(planEvent(s,interior).id,EventId.WindowKnock);
  s.minute=1000;assert.equal(planEvent(s,interior).id,3);
});
test('authored meter-space districts and restore gates match the chapter',()=>{
  assert.equal(districtAt(-12,0),District.Bedroom);
  assert.equal(districtAt(2,0),District.Bedroom);
  assert.equal(districtAt(20.5,9),District.Garden);
  assert.equal(districtAt(-17,32),District.Interior);
  assert.equal(districtAt(80,30),District.Finale);
  assert.equal(districtAt(NaN,0),District.Bedroom);
  assert.equal(canRestoreAt(72,0,Stage.BirthdayRestored),false);
  assert.equal(canRestoreAt(72,0,Stage.RoadUnlocked),true);
  assert.equal(canRestoreAt(71.5,30,Stage.RoadUnlocked),false);
  assert.equal(canRestoreAt(71.5,30,Stage.DarkLampFound),true);
  assert.equal(canRestoreAt(57,0,Stage.Arrival),false);
  assert.equal(canRestoreAt(30,32,Stage.Complete),false);
});
test('browser save roundtrip restores progression and rejects incompatible/corrupt saves',()=>{
  const s=birthday();traverseLoop(s);apply(s,'lamp');
  const player={x:71.5,z:30,yaw:1,pitch:-.3};
  const encoded=serializeSnapshot(s,player), parsed=parseSnapshot(encoded);
  assert.deepEqual(parsed,{state:s,player});
  assert.equal(parseSnapshot('not json'),null);
  for(const change of [d=>d.version++,d=>d.chapter='Unreal',d=>d.state.history[0].count=-1,
    d=>d.player.x=1e12,d=>d.player.pitch=20,d=>d.state.minute=900,d=>d.state.loops=0]){
    const corrupt=JSON.parse(encoded);change(corrupt);assert.equal(parseSnapshot(JSON.stringify(corrupt)),null);
  }
  assert.throws(()=>serializeSnapshot(s,{...player,x:57,z:0}));
});
test('snapshot strips unknown properties instead of copying browser data into live state',()=>{
  const s=createState(), player={x:-12,z:0,yaw:0,pitch:0};
  const data=JSON.parse(serializeSnapshot(s,player));
  data.state.unexpected={inject:true};data.player.unexpected='value';
  const parsed=parseSnapshot(JSON.stringify(data));
  assert.equal('unexpected' in parsed.state,false);assert.equal('unexpected' in parsed.player,false);
});

const nativeHarness=String.raw`
#include "Core/NostalgiaCore.h"
#include <iostream>
#include <iomanip>
using namespace Nostalgia;
bool first=true;
void emit(const char* name,const State& s,EventId id=EventId::Count,bool ok=false){
 if(!first)std::cout<<",";first=false;
 std::cout<<"{\"name\":\""<<name<<"\",\"id\":"<<(int)id<<",\"ok\":"<<(ok?"true":"false")<<",\"state\":{";
 std::cout<<"\"stage\":"<<(int)s.stage<<",\"minute\":"<<s.minute<<",\"elapsed\":"<<s.elapsed<<",\"randomState\":"<<s.randomState<<",\"loops\":"<<s.loops<<",\"openDoors\":"<<(int)s.openDoors<<",\"lastTransition\":"<<s.lastTransition<<",\"lastShock\":"<<s.lastShock<<",\"history\":[";
 for(size_t i=0;i<EventCount;++i){if(i)std::cout<<",";std::cout<<"{\"count\":"<<s.history[i].count<<",\"lastTime\":"<<s.history[i].lastTime<<"}";}
 std::cout<<"]}}";
}
Context at(District d){return {d,false,false,10,100};}
int main(){std::cout<<std::setprecision(17)<<"["; State s;emit("initial",s);
 bool ok=Apply(s,Action::EnterPlayhouse);emit("early-exit",s,EventId::Count,ok);
 ok=Apply(s,Action::Photo);emit("photo",s,EventId::Count,ok);
 Advance(s,1.5);emit("clock",s);
 auto p=PlanEvent(s,at(District::Garden));ok=CommitEvent(s,p,false);emit("failed-audio",s,p.GetEvent(),ok);
 p=PlanEvent(s,at(District::Garden));ok=CommitEvent(s,p,true);emit("audio",s,p.GetEvent(),ok);
 ok=Apply(s,Action::ComparePhoto);emit("compare",s,EventId::Count,ok);
 ok=Apply(s,Action::RestoreBirthday);emit("birthday",s,EventId::Count,ok);
 auto blocked=at(District::Street);blocked.available[1]=false;p=PlanEvent(s,blocked);emit("offscreen",s,p.GetEvent());
 p=PlanEvent(s,at(District::Street));Advance(s,.5);ok=CommitEvent(s,p,true);emit("stale",s,p.GetEvent(),ok);
 p=PlanEvent(s,at(District::Street));ok=CommitEvent(s,p,true);emit("sighting",s,p.GetEvent(),ok);
 p=PlanEvent(s,at(District::Interior));emit("peace",s,p.GetEvent());Advance(s,45);
 p=PlanEvent(s,at(District::Interior));ok=CommitEvent(s,p,true);emit("knock",s,p.GetEvent(),ok);
 ok=TraverseLoop(s);emit("loop",s,EventId::Count,ok);ok=Apply(s,Action::ExamineLamp);emit("lamp",s,EventId::Count,ok);
 ok=Apply(s,Action::EnterPlayhouse);emit("exit",s,EventId::Count,ok);
 auto rules=DefaultRules();for(size_t i=0;i<EventCount;++i){rules[i]=rules[0];rules[i].id=(EventId)i;rules[i].cooldown=0;rules[i].maximumPlays=100;}
 State ties;for(int i=0;i<25;++i){auto e=SelectEvent(ties,at(District::Garden),rules);emit("tie",ties,e,true);}
 std::cout<<"]";
}
`;
test('actual production C++ and JavaScript produce identical quest/director/RNG traces',()=>{
  const repo=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../..');
  const dir=mkdtempSync(path.join(tmpdir(),'nostalgia-parity-'));
  try {
    const source=path.join(dir,'trace.cpp'), binary=path.join(dir,'trace');
    writeFileSync(source,nativeHarness);
    const core=path.join(repo,'Source/Nostalgia/Private/Core');
    execFileSync(process.env.CXX||'g++',['-std=c++20','-O1','-I',path.join(repo,'Source/Nostalgia/Public'),source,
      ...['NostalgiaCore','NostalgiaEncounter','NostalgiaWorldLayout','NostalgiaSnapshot'].map(n=>path.join(core,`${n}.cpp`)),'-o',binary]);
    const native=JSON.parse(execFileSync(binary,{encoding:'utf8'}));
    const trace=[], s=createState();
    const emit=(name,state,id=3,ok=false)=>trace.push({name,id,ok,state:clone(state)});
    emit('initial',s);emit('early-exit',s,3,apply(s,'exit'));emit('photo',s,3,apply(s,'photo'));
    advance(s,1.5);emit('clock',s);
    let p=planEvent(s,at(2));emit('failed-audio',s,p.id,commitEvent(s,p,false));
    p=planEvent(s,at(2));emit('audio',s,p.id,commitEvent(s,p,true));
    emit('compare',s,3,apply(s,'compare'));emit('birthday',s,3,apply(s,'birthday'));
    const blocked=at(1);blocked.available[1]=false;p=planEvent(s,blocked);emit('offscreen',s,p.id);
    p=planEvent(s,at(1));advance(s,.5);emit('stale',s,p.id,commitEvent(s,p,true));
    p=planEvent(s,at(1));emit('sighting',s,p.id,commitEvent(s,p,true));
    p=planEvent(s,at(3));emit('peace',s,p.id);advance(s,45);
    p=planEvent(s,at(3));emit('knock',s,p.id,commitEvent(s,p,true));
    emit('loop',s,3,traverseLoop(s));emit('lamp',s,3,apply(s,'lamp'));emit('exit',s,3,apply(s,'exit'));
    const rules=defaultRules().map((_,i)=>({...defaultRules()[0],id:i,cooldown:0,maximumPlays:100})), ties=createState();
    for(let i=0;i<25;++i){const offer=planEvent(ties,at(2),rules);const ok=commitEvent(ties,offer,true);emit('tie',ties,offer.id,ok);}
    assert.deepEqual(trace,native);
    assert.equal(native.length,41);
  } finally {rmSync(dir,{recursive:true,force:true});}
});
