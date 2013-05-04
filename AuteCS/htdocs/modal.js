var winAlert=window.alert;
 window.alert=function(alert,dr){
 var reValue=null;
 if(dr){
  var obj=document.createElement("span");
      obj.innerHTML=alert;
      obj.style.position="absolute";
      obj.style.fontSize="9pt";
      obj.style.left="0px";
      obj.style.top="0px";
      obj.style.zIndex="9999";
      obj.style.visibility="hidden";
      document.body.appendChild(obj);
  var winWidth=obj.offsetWidth;
  var winHeight=obj.offsetHeight;
  dr.push(alert);
  var rev=window.showModalDialog("/alert.htm?"+Math.random(),dr,"dialogWidth:"+(winWidth)+"px;dialogHeight:"+(winHeight)+"px;"+"status: no;toolbar=no;menubar=no;location=no");
  if(rev!=undefined)reValue=rev;
  if(reValue != null) 
  //document.getElementById("test").value = reValue;
  return reValue;
  }else{
  winAlert(alert);
  }
 return reValue;
 }
