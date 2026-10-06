const titles = [
  {id:'halo3', title:'Halo 3', type:'Game', icon:'III', c1:'#6b7e51', c2:'#20261a'},
  {id:'forza4', title:'Forza Motorsport 4', type:'Game', icon:'F4', c1:'#8b2030', c2:'#17181b'},
  {id:'gears3', title:'Gears of War 3', type:'Game', icon:'G3', c1:'#5e251d', c2:'#171311'},
  {id:'minecraft', title:'Minecraft: Xbox 360 Edition', type:'Game', icon:'▦', c1:'#43894b', c2:'#233218'},
  {id:'skate3', title:'Skate 3', type:'Game', icon:'S3', c1:'#2873a5', c2:'#121c23'},
  {id:'aurora', title:'Aurora', type:'Homebrew', icon:'A', c1:'#9b4bc5', c2:'#211529'},
  {id:'retro', title:'RetroArch 360', type:'Emulator', icon:'∞', c1:'#333b45', c2:'#101214'},
  {id:'xex', title:'XeXMenu', type:'Homebrew', icon:'X', c1:'#1477a7', c2:'#10232e'},
  {id:'codbo2', title:'Call of Duty: Black Ops II', type:'Game', icon:'II', c1:'#9b4a16', c2:'#1f150f'},
  {id:'rdr', title:'Red Dead Redemption', type:'Game', icon:'R', c1:'#ab241b', c2:'#28100d'},
  {id:'portal2', title:'Portal 2', type:'Game', icon:'P2', c1:'#6b9eb9', c2:'#182128'},
  {id:'sonic', title:'Sonic Generations', type:'Game', icon:'S', c1:'#246ec4', c2:'#102240'}
];
const state = {
  favs: new Set(JSON.parse(localStorage.getItem('sd360-favs') || '[]')),
  favOnly: false
};

const $ = s => document.querySelector(s);
const $$ = s => [...document.querySelectorAll(s)];
function persist(){ localStorage.setItem('sd360-favs', JSON.stringify([...state.favs])); }
function toast(msg){ const t=$('#toast'); t.textContent=msg; t.classList.add('show'); setTimeout(()=>t.classList.remove('show'),1600); }
function tile(t){
  const el=document.createElement('div');
  el.className='game-tile'; el.tabIndex=0;
  el.innerHTML=`<button class="fav" title="Favorite">${state.favs.has(t.id)?'★':'☆'}</button><div class="cover" style="--c1:${t.c1};--c2:${t.c2}">${t.icon}</div><div class="game-meta"><b>${t.title}</b><small>${t.type}</small></div>`;
  el.querySelector('.fav').onclick=e=>{ e.stopPropagation(); state.favs.has(t.id)?state.favs.delete(t.id):state.favs.add(t.id); persist(); render(); };
  el.onclick=()=>toast(`Launch adapter: ${t.title}`);
  el.onkeydown=e=>{ if(e.key==='Enter') el.click(); };
  return el;
}
function render(){
  $('#gameCount').textContent=`${titles.length} titles`;
  const recent=$('#recentRow'); recent.innerHTML=''; titles.slice(0,6).forEach(t=>recent.appendChild(tile(t)));
  const q=$('#search').value.toLowerCase();
  let list=titles.filter(t=>t.title.toLowerCase().includes(q));
  if(state.favOnly) list=list.filter(t=>state.favs.has(t.id));
  const grid=$('#libraryGrid'); grid.innerHTML=''; list.forEach(t=>grid.appendChild(tile(t)));
  $('#favOnly').textContent=`Favorites: ${state.favOnly?'on':'off'}`;
}
function showPage(id){
  $$('.page').forEach(p=>p.classList.toggle('active',p.id===id));
  $$('#nav button').forEach(b=>b.classList.toggle('active',b.dataset.page===id));
  if(id==='games') setTimeout(()=>$('#search').focus(),50);
}
$$('[data-page]').forEach(b=>b.onclick=()=>showPage(b.dataset.page));
$$('[data-page-jump]').forEach(b=>b.onclick=()=>showPage(b.dataset.pageJump));
$('#searchJump').onclick=()=>showPage('games');
$('#search').oninput=render;
$('#favOnly').onclick=()=>{state.favOnly=!state.favOnly; render();};
$('#rescan').onclick=()=>toast('Library scan complete · 12 titles');
$('#motion').onchange=e=>document.documentElement.style.setProperty('--motion',e.target.checked?'0s':'.14s');
$('#compact').onchange=e=>document.body.classList.toggle('compact',e.target.checked);
$('#ftp').onchange=e=>toast(e.target.checked?'FTP requested · backend not bound':'FTP disabled');
window.addEventListener('keydown', e=>{
  if(e.key==='Escape') showPage('home');
  if(e.key==='/' && document.activeElement!==$('#search')){ e.preventDefault(); showPage('games'); }
});
render();
const requestedPage = new URLSearchParams(location.search).get('page');
if (requestedPage && document.getElementById(requestedPage)) showPage(requestedPage);
