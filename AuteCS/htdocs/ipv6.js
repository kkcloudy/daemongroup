

function mask(obj,xy){

  	//obj.value=obj.value.replace(/[a-f0-9]/g,'');
  	var oval=obj.value;
  	var vel=/[a-f0-9]/g;
  	var mval=oval.match(vel);
  	
  	if(null==mval){
  		obj.value="";
  	}else if(oval.length!=mval.length){
  		obj.value=oval.substring(0,mval.length);
  	}
  	
 	var key1=event.keyCode;

  	if (key1==37 || key1==39){  /*left,right*/
	
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=key1==37?nextip-1:nextip+1;
	
    	nextip=nextip>=9?8:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=9 && nextip!=0){
			
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
	  		var a = obj.name.substring(0, obj.name.length-1) + 5;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 6;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 7;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 8;
	  		document.getElementById(a).blur();
			
		}
		
  	}else if(key1==190 || key1==110){  //period colon, KP_. KP_Decimal
		
		
		
		if(obj.value==""){
			
			obj.focus();
			
		}else{
			
			obj.blur();
		
    		nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    		nextip=nextip+1;
	
    		nextip=nextip>=9?8:nextip;
    		nextip=nextip<=0?1:nextip;

			if(nextip!=9 && nextip!=0){
			
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
	  			var a = obj.name.substring(0, obj.name.length-1) + 5;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 6;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 7;
	  			document.getElementById(a).blur();
	  			var a = obj.name.substring(0, obj.name.length-1) + 8;
	  			document.getElementById(a).blur();
			
			}
			
		}

	}else if(key1==46){ //delete
		
		obj.value="";
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=nextip+1;
	
    	nextip=nextip>=9?8:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=9 && nextip!=0){
			
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
	  		var a = obj.name.substring(0, obj.name.length-1) + 5;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 6;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 7;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 8;
	  		document.getElementById(a).blur();
			
		}

	}else if(key1==8){   //BackSpace
		
		obj.value="";
    	obj.blur();
		
    	nextip=parseInt(obj.name.substr(obj.name.length-1,1));
    	nextip=nextip-1;
	
    	nextip=nextip>=9?8:nextip;
    	nextip=nextip<=0?1:nextip;

		if(nextip!=9 && nextip!=0){
			
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
	  		var a = obj.name.substring(0, obj.name.length-1) + 5;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 6;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 7;
	  		document.getElementById(a).blur();
	  		var a = obj.name.substring(0, obj.name.length-1) + 8;
	  		document.getElementById(a).blur();
			
		}

	}else if(obj.value.length>=4){

  		if(parseInt(obj.value)>=65535 || parseInt(obj.value)<=0){
			
    		alert(xy);
    		obj.value="";
    		obj.focus();
			obj.select();
			
    		return false
			
  		}else{
			
   			obj.blur();
			
   			nextip=parseInt(obj.name.substr(obj.name.length-1,1));
			nextip=nextip+1;
			
   			nextip=nextip>9?8:nextip;
   			nextip=nextip<=0?1:nextip;
			
   			if(nextip!=9 && nextip!=0){
   			
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
	  			var b = obj.name.substring(0, obj.name.length-1) + 5;
	  			document.getElementById(b).blur();                      
	  			var b = obj.name.substring(0, obj.name.length-1) + 6;
	  			document.getElementById(b).blur();
	  			var b = obj.name.substring(0, obj.name.length-1) + 7;
	  			document.getElementById(b).blur();
	  			var b = obj.name.substring(0, obj.name.length-1) + 8;
	  			document.getElementById(b).blur();
				
			}
			
  		}
		
	}else if(obj.value.length>1){

    		//obj.value=parseInt(obj.value);
    		obj.focus();
		
	}
	
}
