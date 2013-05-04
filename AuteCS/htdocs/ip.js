function mask(obj,xy){


  	obj.value=obj.value.replace(/[^\d]/g,'');
 	key1=event.keyCode;


  	if (key1==37 || key1==39){  /*×óÒÆ¼ü¡¢ÓÒÒÆ¼ü*/
	
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=key1==37?nextip-1:nextip+1;
	
    	nextip=nextip>=5?4:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=5 && nextip!=0){
			
			var a = obj.name.substring(0, obj.name.length-1) + nextip;
			document.getElementById(a).focus();
			document.getElementById(a).select();
			
		}else{

	  		var a = obj.name.substring(0, obj.name.length-1) + 1;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 2;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 3;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 4;
	  		document.getElementById(a).blur();
			
		}
		
  	}else if(key1==190 || key1==110){
		
		
		
		if(obj.value==""){
			
			obj.focus();
			
		}else{
			
			obj.blur();
		
    		nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    		nextip=nextip+1;
	
    		nextip=nextip>=5?4:nextip;
    		nextip=nextip<=0?1:nextip;

			if(nextip!=5 && nextip!=0){
			
				var a = obj.name.substring(0, obj.name.length-1) + nextip;
				document.getElementById(a).focus();
				document.getElementById(a).select();
			
			}else{

	  			var a = obj.name.substring(0, obj.name.length-1) + 1;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 2;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 3;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 4;
	  			document.getElementById(a).blur();
			
			}
			
		}

	}else if(key1==46){
		
		obj.value="";
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=nextip+1;
	
    	nextip=nextip>=5?4:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=5 && nextip!=0){
			
			var a = obj.name.substring(0, obj.name.length-1) + nextip;
			document.getElementById(a).value="";
			document.getElementById(a).focus();
			document.getElementById(a).select();
			
		}else{

	  		var a = obj.name.substring(0, obj.name.length-1) + 1;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 2;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 3;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 4;
	  		document.getElementById(a).blur();
			
		}

	}else if(key1==8){
		
		obj.value="";
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=nextip-1;
	
    	nextip=nextip>=5?4:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=5 && nextip!=0){
			
			var a = obj.name.substring(0, obj.name.length-1) + nextip;
			var v= obj.name.substring(0, obj.name.length-1) + (nextip+1);
			document.getElementById(v).value="";
			document.getElementById(a).focus();
			document.getElementById(a).select();
			
		}else{

	  		var a = obj.name.substring(0, obj.name.length-1) + 1;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 2;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 3;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 4;
	  		document.getElementById(a).blur();
			
		}

	}else if(obj.value.length>=3){

  		if(parseInt(obj.value)>=256 || parseInt(obj.value)<=0){
			
    		alert(xy);
    		obj.value="";
    		obj.focus();
			obj.select();
			
    		return false
			
  		}else{
			
   			obj.blur();
			
   			nextip=parseInt(obj.name.substr(obj.name.length-1,1))+1;
			
   			nextip=nextip>5?4:nextip;
   			nextip=nextip<=0?1:nextip;
			
   			if(nextip!=5 && nextip!=0){
   
      			var b = obj.name.substring(0, obj.name.length-1) + nextip;
	  			document.getElementById(b).focus();
	  			document.getElementById(b).select();
				
			}else{

	  			var b = obj.name.substring(0, obj.name.length-1) + 1;
	  			document.getElementById(b).blur();
	  			var b = obj.name.substring(0, obj.name.length-1) + 2;
	  			document.getElementById(b).blur();
	  			var b = obj.name.substring(0, obj.name.length-1) + 3;
	  			document.getElementById(b).blur();
	  			var b = obj.name.substring(0, obj.name.length-1) + 4;
	  			document.getElementById(b).blur();
				
			}
			
  		}
		
	}else if(obj.value.length>1){

    		obj.value=parseInt(obj.value);
    		obj.focus();
		
	}
	
}

function mask_c(){

  	clipboardData.setData('text',clipboardData.getData('text').replace(/[^\d]/g,''));
}

function showTR(myTr,myTr2)
{
  var q=document.getElementsByName("encry_type");
  var len=q.length;
  for(var i=0;i<len;i++)
  {
    if(q[i].checked==true)
    {
      if(q[i].value=='none')
	{
        	myTr.style.display='none';
       		myTr2.style.display='none'; 
	} 
	else
	{
      		myTr.style.display='block'; 
		myTr2.style.display='block';	
	}    
  break;
    }
  }
}

function show_ae_passwd_tr(obj)
{
  var index = obj.selectedIndex;
  var v = obj.options[index].value;  
  var patrn=/[.p12]$/; 
  if(patrn.exec(v)) 
  {
  	ae_passwd_tr.style.display='block'; 
  }
  else
  {
  	ae_passwd_tr.style.display='none'; 
  }
}