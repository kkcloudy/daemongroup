// JScript 文件

var alternateFrame=null;//生成的iframe
var alternateWin=null;

window.alert=showAlert;
window.confirm=showConfirm;

/**//**
 * 人机交互窗口，覆盖自带的
 */
function alternateWindow(){
this.win=null;//生成对话框的窗口对象
this.pBody=null;//生成的body容器对象
this.pBg=null;
this.type="alert";//默认的种类是alert
this.FocusWhere="OK";//焦点在哪个按钮上
}
/**//**
 * 模仿的alert窗口
 */
function showAlert(info,ok,des){
alternateWin=new alternateWindow();
var pBody = alternateWin.init();
alternateWin.initAlertBody(pBody,info,ok);
alternateWin.type="alert";
}
  /**//**
 * 模仿的Confirm窗口
 */
function showConfirm(info,ok_func,notok_func,ok_str,not_okstr){
alternateWin=new alternateWindow();
var pBody = alternateWin.init();
alternateWin.initConfirmBody(pBody,info,ok_func,notok_func,ok_str,not_okstr);
alternateWin.type="confirm";
}
/**//**
 * 作用：初始基本信息
 */
alternateWindow.prototype.init=function() {
if(alternateFrame==null){
alternateFrame=document.createElement("<iframe allowTransparency='true' id='popframe' frameborder=0 marginheight=0 src='about:blank' marginwidth=0 hspace=0 vspace=0 scrolling=no></iframe>")
alternateFrame.style.position="absolute";
document.body.appendChild(alternateFrame);
}else{
alternateFrame.style.visibility="visible";
}
alternateFrame.style.width=screen.availWidth;
alternateFrame.style.height=screen.availHeight;
alternateFrame.style.left=document.body.scrollLeft;
alternateFrame.style.top=document.body.scrollTop;
alternateFrame.name=alternateFrame.uniqueID;

this.win=window.frames[alternateFrame.name];
this.win.document.write("<body leftmargin=0 topmargin=0 oncontextmenu='self.event.returnValue=false'><div id=popbg></div><div id=popbody></div><div></div></body>");
this.win.document.body.style.backgroundColor="transparent";
document.body.style.overflow="hidden";
this.pBody=this.win.document.body.children[1];
this.pBg=this.win.document.body.children[0];
this.hideAllSelect();
this.initBg();

return this.pBody;
}

 /**//**
* 作用：初始化背景层
  */
alternateWindow.prototype.initBg=function(){
with(this.pBg.style){
position="absolute";
left="0";
top="0";
width="100%";
height="100%";
visibility="hidden";
backgroundColor="#ECE9D8";
filter="blendTrans(duration=1) alpha(opacity=0)";
}
this.pBg.filters.blendTrans.apply();
this.pBg.style.visibility="visible";
this.pBg.filters.blendTrans.play();
}
/**//**
 * 作用：初始化显示层
 */
alternateWindow.prototype.initAlertBody=function(obj,info,ok){
with(obj.style){
position="absolute";
width="250";
height="100";
backgroundColor="#FBFDFF";
}
if(ok==null){
ok="确定";
}
obj.style.left=window.document.body.clientWidth/2-200;
obj.style.top=window.document.body.clientHeight/3;
var str;

str="<table width=250 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td>";
str+="<div align=center>";
str+="<table border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";


str+="<td><table width=258 height=37 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=231 height=37><img src=/images/alert/error_1_1.gif width=231 height=37/></td>";
str+="<td>";
str+="<input type='button' onkeydown='parent.alternateWin.onKeyDown(event,this)'";
str+="  onclick='parent.alternateWin.closeWin()' ";
str+="  style='width:27px;height:37px;background:url(/images/alert/error_1_2.gif)  left top no-repeat; border=0'>";
str+="</td>";
str+="</tr></table></td>";

str+="</tr>";
str+="<tr>";
str+="<td><table width=258 height=63 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=58><img src=/images/alert/error_2_1.gif width=58 height=63/></td>";
str+="<td width=190 bgcolor=#ece9d8 nowrap><div align=left><span class=STYLE3>";
str+=info+"</span></div></td>";
str+="<td width=10><img src=/images/alert/error_2_2.gif width=10 height=63/></td>";
str+="</tr>";
str+="</table></td>";
str+="</tr>";
str+="<tr>";
str+="<td><table width=258 height=31 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=17 style='background:url(/images/alert/error_3_1.gif) repeat-y';></td>";
str+="<td width=221 bgcolor=#ece9d8><div align=center>";
str+="<input type='button'  value='"+ok+"' id='OK'" +
     " onkeydown='parent.alternateWin.onKeyDown(event,this)'"+
     " onclick='parent.alternateWin.closeWin()' style='border:1px   solid   black;background:#F4F4F0;width:50 '>" ;
str+="</div></td>";
str+="<td width=20 style='background:url(/images/alert/error_3_2.gif) repeat-y';></td>";
str+="</tr>";
str+="</table></td>";
str+="</tr>";
str+="<tr>";
str+="<td width=258 height=10><img src=/images/alert/error_4.gif width=258 height=10/></td>";
str+="</tr>";
str+="</table>";
str+="</div>";
str+="</td>";
str+="</tr>";
str+="</table>";
     
     
     
obj.innerHTML=str;
this.win.document.body.all.OK.focus();
this.FocusWhere="OK";
}

alternateWindow.prototype.onKeyDown=function(event,obj){
  switch(event.keyCode){
  case 9:
   event.keyCode=-1;
  if(this.type=="confirm"){
  if(this.FocusWhere=="OK"){
  this.win.document.body.all.NO.focus();
  this.FocusWhere="NO";
  }else{
  this.win.document.body.all.OK.focus();
  this.FocusWhere="OK";
  }
  }
  break;
  case 13:obj.click();;break;
  case 27:this.closeWin();break; 
  }

  }
/**//**
 * 作用：初始化显示层 conFirm提示层
 */
alternateWindow.prototype.initConfirmBody=function(obj,info,ok_func,notok_func,ok_str,notok_str){
with(obj.style){
position="absolute";
width="250";
height="100";
backgroundColor="#FBFDFF";
}
if(ok_str==null){
ok_str="确定";
}
if(notok_str==null){
notok_str="取消"
}
obj.style.left=window.document.body.clientWidth/2-200;
obj.style.top=window.document.body.clientHeight/3;
var str;



str="<table width=250 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td>";
str+="<div align=center>";
str+="<table border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";



str+="<td><table width=258 height=37 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=231 height=37><img src=/images/alert/error_1_1.gif width=231 height=37/></td>";
str+="<td>";
str+="<input type='button' onkeydown='parent.alternateWin.onKeyDown(event,this)'";
str+="  onclick='parent.alternateWin.closeWin()' ";
str+="  style='width:27px;height:37px;background:url(/images/alert/error_1_2.gif)  left top no-repeat; border=0'>";
str+="</td>";
str+="</tr></table></td>";

str+="</tr>";
str+="<tr>";
str+="<td><table width=258 height=63 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=58><img src=/images/alert/error_2.gif width=58 height=63/></td>";
str+="<td width=190 bgcolor=#ece9d8 nowrap><div align=left><span class=STYLE3>";
str+=info+"</span></div></td>";
str+="<td width=10><img src=/images/alert/error_2_2.gif width=10 height=63/></td>";
str+="</tr>";
str+="</table></td>";
str+="</tr>";
str+="<tr>";
str+="<td><table width=258 height=31 border=0 cellspacing=0 cellpadding=0>";
str+="<tr>";
str+="<td width=17 style='background:url(/images/alert/error_3_1.gif) repeat-y';></td>";
str+="<td width=221 bgcolor=#ece9d8><div align=center>";

str+="<input type='button' id='OK'" ;
str+=" onkeydown='parent.alternateWin.onKeyDown(event,this)'";
str+=" onclick='parent.alternateWin.closeWin();parent."+ok_func+"();' " ;
str+=" value='"+ok_str+"' style='border:1px   solid   black;background:#F4F4F0;width:50 '>";
str+="&nbsp;&nbsp;&nbsp;<input type='button' value='"+notok_str+"' id='NO'";
str+=" onkeydown='parent.alternateWin.onKeyDown(event,this)'";
str+=" onclick='parent.alternateWin.closeWin();" ;
str+=" parent."+notok_func+"();' style='border:1px   solid   black;background:#F4F4F0;width:50 '>";
     
str+="</div></td>";
str+="<td width=20 style='background:url(/images/alert/error_3_2.gif) repeat-y';></td>";
str+="</tr>";
str+="</table></td>";
str+="</tr>";
str+="<tr>";
str+="<td width=258 height=10><img src=/images/alert/error_4.gif width=258 height=10/></td>";
str+="</tr>";
str+="</table>";
str+="</div>";
str+="</td>";
str+="</tr>";
str+="</table>";




obj.innerHTML=str;
this.win.document.body.all.OK.focus();
}

/**//**
 * 作用：关闭一切
 */
alternateWindow.prototype.closeWin=function(){
alternateFrame.style.visibility="hidden";
this.showAllSelect();
document.body.style.overflow="auto";
}
/**//**
  * 作用:隐藏所有的select
  */
alternateWindow.prototype.hideAllSelect=function(){
  var obj;
  obj=document.getElementsByTagName("SELECT");
  var i;
  for(i=0;i<obj.length;i++)
obj[i].style.visibility="hidden";
  }
/**//**
 * 显示所有的select
 */ 
  alternateWindow.prototype.showAllSelect=function(){
  var obj;
  obj=document.getElementsByTagName("SELECT");
  var i;
  for(i=0;i<obj.length;i++)
obj[i].style.visibility="visible";
}   