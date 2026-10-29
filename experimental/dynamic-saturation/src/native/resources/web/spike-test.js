// QA probes are inert unless the isolated native host opts in.
window.apRunProbe=()=>{
// Isolated native bridge probes; no audio execution or changes to approved rendering.
window.apParamValues={};const apOriginalSPVFD=window.SPVFD;window.SPVFD=(i,v)=>{window.apParamValues[i]=v;apOriginalSPVFD(i,v);};
window.apProbe=()=>({nativeFault:window.apNativeFault,leds:[...document.querySelectorAll("[data-led-key]")].map(x=>({key:x.dataset.ledKey,brightness:Number(x.dataset.brightness),available:x.dataset.meterAvailable})),title:document.title,images:[...document.images].map(i=>({src:i.src,ready:i.complete})),pose:document.querySelector('[data-pig-state]')?.getAttribute('data-pig-state'),knobs:document.querySelectorAll('[role=slider]').length,params:window.apParamValues,sliders:[...document.querySelectorAll('[role=slider]')].map(x=>({name:x.getAttribute('aria-label'),value:x.getAttribute('aria-valuenow')})),pigReady:document.querySelector('[data-pig-ready]')?.getAttribute('data-pig-ready'),errors:window.apErrors||[]});
window.addEventListener('error',e=>(window.apErrors??=[]).push(e.message));
function report(stage){const data=JSON.stringify({stage,...window.apProbe()});IPlugSendMsg({msg:'SAMFUI',msgTag:901,ctrlTag:0,data:btoa(String.fromCharCode(...new TextEncoder().encode(data)))});}
window.apCapture=report;
setTimeout(()=>{report('ready');document.querySelector('[role=slider][aria-label=Fatness]').dispatchEvent(new KeyboardEvent('keydown',{key:'End',bubbles:true}));},800);
setTimeout(()=>document.querySelector('button[type=button]').click(),1200);
setTimeout(()=>report('ui-change'),1600);
setTimeout(()=>report('host-change'),6000);

};
