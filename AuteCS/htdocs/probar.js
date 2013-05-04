
function test(textobj)
{

var sWidth,sHeight; 
sWidth=document.body.offsetWidth; 
sHeight=screen.height;  
var bgObj=document.createElement("<iframe allowTransparency='true' id='popframe' frameborder=0 marginheight=0 src='about:blank' marginwidth=0 hspace=0 vspace=0 scrolling=no></iframe>");
bgObj.setAttribute('id','bgDiv'); 
bgObj.style.position="absolute";    
   
 

bgObj.style.opacity="40%"; 
bgObj.style.zIndex = "10000";   
bgObj.style.width=screen.availWidth;
bgObj.style.height=screen.availHeight;
bgObj.style.left=document.body.scrollLeft;
bgObj.style.top=document.body.scrollTop;

document.body.appendChild(bgObj);  
document.body.style.overflow="hidden";

var msgBox=document.createElement("div");
msgBox.setAttribute("id","msgBox1");
msgBox.setAttribute("align","center");
msgBox.style.background="#ECE9D8"; 
msgBox.style.border="2px solid buttonface";
msgBox.style.position = "absolute"; 
msgBox.style.left = "50%"; 
msgBox.style.top = "50%"; 
msgBox.style.font="14px/1.6em Verdana, Geneva, Arial, Helvetica, sans-serif"; 
msgBox.style.marginLeft = "-225px" ; 
msgBox.style.marginTop = -75+document.documentElement.scrollTop+"px"; 
//if(page=="write"){msgh=300;msgw=500;}
msgBox.style.width = "300px"; 
msgBox.style.height ="70px";   
msgBox.style.textAlign = "center"; 
msgBox.style.lineHeight ="25px"; 
msgBox.style.zIndex = "10001";  

var title=document.createElement("h4"); 
title.setAttribute("id","msgTitle"); 
title.setAttribute("align","right"); 
title.style.margin="0"; 
title.style.padding="3px";  
title.style.filter="progid:DXImageTransform.Microsoft.Alpha(startX=20, startY=20, finishX=100, finishY=100,style=1,opacity=75,finishOpacity=100);"; 
title.style.opacity="75%"; 
title.style.background="#217CF6"; 
title.style.border="1px solid buttonface"; 
title.style.height="18px"; 
title.style.font="12px Verdana, Geneva, Arial, Helvetica, sans-serif"; 
title.style.color="white";
title.style.cursor="pointer"; 
//var guanbi=document.createElement("strong");
//guanbi.setAttribute("id","close"); 
// guanbi.innertHTML="¹Ø±Õ"; 
//title.onclick=function(){
//  document.body.removeChild(bgObj); 
// document.body.removeChild(msgBox); 
//       document.body.removeChild(title);
//       document.body.removeChild(guanbi);
// }
// title.innerHTML="¹Ø±Õ"; 

document.body.appendChild(msgBox); 
var txt=document.createElement("p"); 
txt.style.margin="1em 0" 
txt.setAttribute("id","msgTxt");  
txt.innerHTML= textobj+"....."; 
//document.getElementById("msgBox1").appendChild(title); 
document.getElementById("msgBox1").appendChild(txt);   
} 
