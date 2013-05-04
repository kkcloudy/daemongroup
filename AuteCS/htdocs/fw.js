


function selectObj()
{
	this.selected=0;
	this.items=new Array();
	
	this.setSelected = function( selected ){
		this.selected = selected;	
	}
	
	this.setName = function (urname){
		this.urname=urname;
	}
			
	this.setId = function (urid){
		this.urid=urid;
	}

	this.setClass = function (classtype){
		this.classtype=classtype;	
	}
		
	this.addItem = function (value,show){
		var curNum = this.items.length;
		
		this.items[curNum]=new Array();
		this.items[curNum]["value"]=value;
		this.items[curNum]["show"]=show;
	}

	this.setOnSelChangeFunc = function( func ){
		this.onSelChangeFunc=func;
	}
	
	this.writeHtml = function(){
		document.write( "<select id=\"" + this.urid + "\" name=\"" + this.urname + "\" class=\"" + this.classtype + "\" onchange=\"" + 
								this.onSelChangeFunc + "\">" );
								
		for( var i=0; i < this.items.length; i++ )
		{
			document.write( "<option value=\"" + this.items[i]["value"] );
			if( i == this.selected )
			{
				document.write( "\" selected=\"selected\"" );
			}
			else
			{
				document.write( "\"" );
			}
			document.write( ">" + this.items[i]["show"] + "</option>" );			
		}
		
		document.write("</select>");
	}
	
}



function tbCfgInfo()
{
	
	this.col=new Array();

	this.col[0] = new Array();//"ruleId";
	this.col[0][0] = "ruleId";
	this.col[0][1] = "Position";
	this.col[0][2] = "90px";
	
	this.col[1] = new Array();//"ruleType";
	this.col[1][0] = "rulename";
	this.col[1][1] = "Rule|Rule Detail";
	this.col[1][2] = "360px";
	
	this.col[2] = new Array();//"ruleType";
	this.col[2][0] = "ruleaction";
	this.col[2][1] = "Action";
	this.col[2][2] = "80px";
	
	this.col[3] = new Array();
	this.col[3][0] = "enable";
	this.col[3][1] = "Disabled";
	this.col[3][2] = "120px";
/*	
	this.col[3] = new Array();
	this.col[3][0] = "";
	this.col[3][1] = "";
*/	
	this.colNum=this.col.length;
	
	this.setShowOfIndex = function ( index, show ){
		this.col[index][1] = show;		
	}
}



function tableBegin()
{
	//style='border-collapse:collapse;' 将相邻表格的边框设置成同一个，也及时两个相邻的边框使用相同的边线，否则在显示tr颜色是，两个
	//边框之间的颜色会不是tr的颜色，不好看
	return "<table border='0' style='border-collapse:collapse;'>";
}



function tableHead( ruleType, cfginfo )
{
//	var cfginfo = new tbCfgInfo();
	var colNum = cfginfo.colNum;
	var ret;
	var i;
	
	ret = "<tr height=25 bgcolor=#eaeff9>";
	ret += "<th width=12></th>";
	for( i=0; i< colNum; i++ ){
		
		ret += "<th width=" + cfginfo.col[i][2] +  " align=center" + ">";
		//ret += ("col"+i);
		if( cfginfo.col[i][0]!='ruleaction' || ruleType == '0' )
			ret += cfginfo.col[i][1];
		else
			ret += '';
		ret += "</th>";
	}
	//修改，删除
	ret += "<th width=42></th>";
	ret += "<th width=48></th>";
	ret += "</tr>";
	return ret;
}




function tableEnd()
{
	return "</table>";
}


function procDetail(obj,showid,rule)
{
	
	var ruleDetailShow = "<table border=0 bgcolor='#F0F0FF' width=100%>";
	ruleDetailShow += "<tr height=1><th width=150px></th><th width=130px></th><th></th></tr>"
	ruleDetailShow += "<tr height=20><td></td><td>Incoming interface:</td><td>" + rule.ineth + "</td></tr>";
	if (rule.ruleType != "3")
		ruleDetailShow += "<tr height=20><td></td><td>Outgoing interface:</td><td>" + rule.outeth + "</td></tr>";
	ruleDetailShow += "<tr height=20><td></td><td>Source address:</td><td>" + rule.srcIpAddr + "</td></tr>";
	ruleDetailShow += "<tr height=20><td></td><td>Destination address:</td><td>" + rule.dstIpAddr + "</td></tr>";
	var protocolStr;
	switch( parseInt(rule.protocl) )
	{
		case 1:
			protocolStr = 'TCP';
			break;
		case 2:
			protocolStr = 'UDP';
			break;
		case 3:
			protocolStr = 'TCP and UDP';
			break;
		case 4:
			protocolStr = 'ICMP';
			break;
		case 0:
		default:
			protocolStr = 'any';
			break;	
	}
	ruleDetailShow += "<tr height=20><td></td><td>Protocol:</td><td>" + protocolStr + "</td></tr>";
	
	var PortStr;
	PortStr = rule.srcPort;
	if( PortStr == '(null)' || null == PortStr )
		PortStr = 'any';
	ruleDetailShow += "<tr height=20><td></td><td>Source port:</td><td>" + PortStr + "</td></tr>";
	
	PortStr = rule.desPort;
	if( PortStr == '(null)' || null == PortStr )
		PortStr = 'any';
	ruleDetailShow += "<tr height=20><td></td><td>Destination port:</td><td>" + PortStr + "</td></tr>";
	
	
	var PkgState = rule.pkg_state;
	if( rule.pkg_state == null || rule.pkg_state == '(null)' )
		PkgState = 'any';
	
	var NATIpAddr = rule.natIpAddr;
	if( rule.natIpAddr == null || rule.natIpAddr == '(null)' )
		NATIpAddr = 'any';
	
	var NATPort = rule.natPort;
	if( rule.natPort == null || rule.natPort == '(null)' )
		NATPort = 'any';

	var StrFilter = rule.string_filter;
	if( StrFilter == null || StrFilter == '(null)' )
		StrFilter = '';
	
	if( rule.ruleType == "0" || rule.ruleType == "3")//wall or input
	{
		//state
		ruleDetailShow += "<tr height=20><td></td><td>State:</td><td>" + PkgState + "</td></tr>";
		//string
		ruleDetailShow += "<tr height=20><td></td><td>String filter:</td><td>" + StrFilter + "</td></tr>";
	}
	
	else if( rule.ruleType == '1' )//dnat
	{
		ruleDetailShow += "<tr height=20><td></td><td>To Destination :</td><td>" + NATIpAddr + "</td></tr>";
		ruleDetailShow += "<tr height=20><td></td><td>To Destination Port:</td><td>" + NATPort + "</td></tr>";
	}
	else
	{
		ruleDetailShow += "<tr height=20><td></td><td>To Source :</td><td>" + NATIpAddr + "</td></tr>";
		ruleDetailShow += "<tr height=20><td></td><td>To Source Port:</td><td>" + NATPort + "</td></tr>";
	}

	ruleDetailShow += "</table>";

	
	if( obj.innerHTML == '+' )
	{
		obj.innerHTML = '-';
//		alert( showid );
//		alert( document.getElementById(showid).name );
		if( rule.ruleType == "0" )
		{
			document.getElementById(showid).parentNode.parentNode.height = 160;

		}
		if( rule.ruleType == "3" )
		{
			document.getElementById(showid).parentNode.parentNode.height = 150;

		}
		else
		{
			document.getElementById(showid).parentNode.parentNode.height = 180;	
		}
		document.getElementById(showid).innerHTML = ruleDetailShow;			
	}
	else
	{
		obj.innerHTML = '+';
		//document.getElementById(showid).parentNode.parentNode.height = 0;
		document.getElementById(showid).parentNode.parentNode.height = 1;
		document.getElementById(showid).innerHTML = "";
	}
	
	autoCheckHeight();
}

function ruleIndexSelectChange( selObj, encry, ruleType )
{
	var oldIndex = selObj.id;
	var newIndex = selObj.value;
	
	if( oldIndex != newIndex )
	{
		var openUrl = "wp_fwrulemodify.cgi?UN="+encry+"&ruleType="+ruleType+"&oldIndex="+oldIndex+"&newIndex="+newIndex+"&temp="+Date.parse(new   Date());
		//如果当前url和要打开的url是一个地址，ie浏览器会不给服务器发送消息，所以这里当发现要打开的url和当前页面的url一致，
		//将参数的位置换下，这样ie就不会阻难该链接了
		//2008-7-26 10:44:39   已经在页面中关闭了ie的缓存，下面语句也就用不着了
		//if( -1 != document.URL.indexOf(openUrl) )
		//{
			//alert( "url is the sam chang it!!!" );
		//	openUrl = "wp_fwrulemodify.cgi?UN="+encry+"&oldIndex="+oldIndex+"&newIndex="+newIndex+"&ruleType="+ruleType;
		//}
		window.open( openUrl, "mainFrame");	
	}
	
	return true;
}


function ruleIndexSelector( allNum, curSelectNum, ruleType, encry )
{
	var retStr;
	
	retStr = "<select id="+curSelectNum+" onchange='ruleIndexSelectChange(this,\"" + encry + "\", \""+ruleType+"\" );'>";
	
	for( var i = 1; i <= allNum; i++ )
	{
		retStr += "<option value='"+ i + "'";
		if( i == curSelectNum )
		{
			retStr += "selected='selected'";
		}
		retStr += ">" + i + "</option>";
	}
	
	retStr += "</select>";
	
	return retStr;
}

function StrArray()
{
	this.allow='allow';
	this.deny='deny';
	this.reject='reject';
	this.disabled='disabled';
	this.enabled='enabled'
	this.state='state'
	this.edit='edit';
	this.del='delete';
	
	this.setAttr = function( attr, value )
	{
		this[attr]=value;	
	}
}
function toraw( lineNum, ruleAllNum, ruleType, encry, cfginfo, strArray )
{
//	var cfginfo = new tbCfgInfo();
	var colNum = cfginfo.colNum;
	var ret;
	var i;
	
	ret = "<tr height=25"
	
//	alert( this.rulestatus );
	if( '0' == this.rulestatus )
	{
		if( 0 == lineNum%2 )
			ret += " class="+"even"+">";
		else
			ret += " class="+"odd"+">";
	}
	else if( '1' == this.rulestatus )
	{
		ret += " class="+"changed"+">";
	}
	else
	{
		ret += " class="+"new"+">";
	}
	//添加一个＋号，用于展开详情
	//fwrules是页面中存放所有规则的数组
	var ruleId = parseInt(lineNum) - 1;
	//alert('test');
	ret += "<td><div style='cursor:hand;background-color:#eeeeff;align:middle;' onclick='procDetail(this,\"rule"+ruleType+lineNum+"\",fwrules"+ruleType+"["+ruleId+"]);'>+</div></td>";
	
	for( i=0; i< colNum; i++ ){
		ret += "<td width=" + cfginfo.col[i][2] + " align=center" + ">";
		if( cfginfo.col[i][0] == "ruleId" )
		{
			ret += ruleIndexSelector(ruleAllNum,lineNum,ruleType,encry);
		}
		else if( cfginfo.col[i][0] == "ruleaction" )
		{
			//如果rule不是防火墙这个格子显示为空
			//if( this.ruleType != '0' )
			if( this.ruleType == '1' || this.ruleType == '2' )
			{
			//	if( this.ruleType == '2' )
			//		ret += 'SNAT';
			//	else
					ret += '';
			}
			else
			{
				if( '0' == this[cfginfo.col[i][0]] )
				{
					ret += strArray.allow;
				}
				else if( '1' == this[cfginfo.col[i][0]] )
				{
					ret += strArray.deny;
				}
				else
				{
					ret += strArray.reject;
				}
			}
		}
		else if( cfginfo.col[i][0] == "enable" )
		{
			if( '0' == this[cfginfo.col[i][0]] )	
			{
				ret +=strArray.disabled;
			}
			else
			{
				ret +=strArray.enabled;
			}
		}
		else
		{
			ret += this[cfginfo.col[i][0]];// + "  " + cfginfo.col[i][0];
		}
		ret += "</td>";
	}
	//添加编辑按钮
	ret += "<td><a href='wp_fwruleedit.cgi?UN="+encry+"&ruleID=" + lineNum + "&ruleNum=" + ruleAllNum + "&ruleType="+ ruleType + "&editType=edit" + "'>"+strArray.edit+"</a></td>"
	
	//添加del按钮
	ret += "<td><a href='wp_fwrulemodify.cgi?UN="+encry+"&delRuleIndex=" + lineNum + "&ruleNum=" + ruleAllNum + "&ruleType="+ ruleType +"'>"+strArray.del+"</a></td>"
	 
	ret += "</tr>";
	
	//显示详情的东西，初始是是不显示的,显示规则的表格共有7列，所以  colspan='7'
	ret += "<tr height=1><td colspan='7'><div id='rule"+ruleType+lineNum+"'></div></td></tr>";
	
	return ret;
}




function ruleDetail(  ){
		var ret,attr;
		
		ret = "<div style=\"position:relative; z-index:1; display:block\" align=center>"
		ret += "<table><tr><th>key</th><th>value</th><tr>";
		for( attr in this ){
			if( typeof this[attr] != "function" )
				ret += ("<tr><td>" + attr.toString() + "</td><td>"+ this[attr]+"</td></tr>" );
		}
		ret += "</div>"
		return ret;
}





function fwrule( ruleType, ruleId, orderNum, rulename, enable, rulestatus, comment, ineth, outeth, srcIpType,
				srcIpAddr, dstIpType, dstIpAddr, protocl, srcPortType, srcPort, desPortType, desPort,
				 ruleaction, natIpType, natIpAddr, natPortType, natPort, pkg_state, string_filter )
{

	this.ruleType = ruleType;
	this.ruleId = ruleId;
	this.orderNum = orderNum;
	this.rulename = rulename;
	this.enable = enable; 
	this.rulestatus = rulestatus;
	this.comment = comment;
	this.ineth = ineth; 
	this.outeth = outeth; 
	this.srcIpType = srcIpType;
	this.srcIpAddr = srcIpAddr;
	this.srcIpAddr = srcIpAddr;
	this.dstIpType = dstIpType;
	this.dstIpAddr = dstIpAddr;
	this.protocl = protocl;
	this.srcPortType = srcPortType;
	this.srcPort = srcPort;
	this.desPortType = desPortType;
	this.desPort = desPort;
	this.ruleaction = ruleaction;
	this.natIpType = natIpType;
	this.natIpAddr = natIpAddr;
	this.natPortType = natPortType;
	this.natPort = natPort;
	this.pkg_state = pkg_state;
	this.string_filter = string_filter;
	
	this.toraw = toraw;
	this.detail = ruleDetail;
}




function ruleIndexSelectOption( num ){
	var ret;
	var i;
//	num = 2;
//	ret = "<select>";
//	for( i = 1; i < num+1; i++ )
	//{
	//	ret += "<option value=" + i + "\">" + i + "</option>   ";
	//}
	
	//ret += "<option value=\""+1+"\" selected=\"selected\">"+1+"</option>";

	//ret = "<select><option value=\"1\" selected=\"selected\">1</option></select>";
	
	//ret = "</select>";
	ret = "<select>"
	for( i = 1; i < num+1; i++ ){
		ret += "<option value=\""+i+"\">"+i+"</option>"
	}
	ret += "</select>";
	
	return ret;
}





function radioGroup()
{
	this.radios = new Array();
	this.relatives = new Array();
	
	this.addItem = function( itemID )
	{
		for( var i = 0; i<this.radios.length;i++ )
		{ 
			if( itemID == this.radios[i] )
			{
				break;	
			}
		}
		if( i == this.radios.length )
		{
			this.radios[this.radios.length] = itemID;
			this.relatives[itemID] = new Array();
		}
	}

	this.bindRelativeItemToRadio = function ( radioID, relativeID )
	{
		var curLength = this.relatives[radioID].length;
		this.relatives[radioID][curLength] = relativeID;
	}
	
	this.changeRelativesStatus = function ( radioID, statu )
	{
		for( var i = 0; i < this.relatives[radioID].length; i++ )
		{
			var obj;
			
			obj = document.getElementById( this.relatives[radioID][i] ) ;
			//alert( obj.name );					
			if(  null != obj && obj.tagName == "INPUT" && obj.type == "text" )
			{
				obj.disabled = statu;
				if( statu == true )//灰显
				{
					obj.style.backgroundColor = '#ccc';
				}
				else
				{
					obj.style.backgroundColor = '#fff';
				}
			}
			else if( null != obj && obj.tabName == "span" )
			{
				if( statu == true )
					obj.innerHTML="";
			}
		}
	}
	
	this.setActive = function( itemID )
	{
		for( var i = 0; i < this.radios.length; i++ )
		{
			if( itemID == this.radios[i] )
			{
				if( null != document.getElementById( this.radios[i] ) )
				{
					document.getElementById( this.radios[i] ).checked = true;
					this.changeRelativesStatus( this.radios[i], false );
				}
				else
				{
					//alert( this.radios[i] +"    1111" );
				}
			}
			else
			{
				if( null != document.getElementById( this.radios[i] ) )
				{
					document.getElementById( this.radios[i] ).checked = false;
					this.changeRelativesStatus( this.radios[i], true );
				}
				else
				{
					//alert( this.radios[i] +"    2222" );	
				}
			}
		}	
	}
}







function   getL(e){  
	var   l=e.offsetLeft;  
	while(e=e.offsetParent)l+=e.offsetLeft;  
	return   l;
}  





function   getT(e){  
	var   t=e.offsetTop;  
	while(e=e.offsetParent)t+=e.offsetTop;  
	return   t ;
}





function	combox(obj,select){  
	this.obj=obj;
	this.name=select;  
	this.select=document.getElementsByName(select)[0]; 
	this.text='abc';
	this.value='asfsf';
}  




combox.prototype.init=function(){  
	var inputbox="<input   name='combox_"+this.name+"'   onchange='"+this.obj+".find()'   "  
			inputbox+="style='position:absolute;width:"+(this.select.offsetWidth-18)+";height:"+(this.select.offsetHeight)+";left:"+getL(this.select)+";top:"+getT(this.select)+"'>" ;

	document.write(inputbox);
	with(this.select.style){ 
		left=getL(this.select);
		top=getT(this.select);
		position="absolute";
		clip="rect(0   "+(this.select.offsetWidth)+"   "+this.select.offsetHeight+"   "+(this.select.offsetWidth-18)+")" ; 
	}
	this.select.onchange=new   Function(this.obj+".change()") ;
	this.change();
}  


////////对象事件定义///////  
combox.prototype.find=function(){  
	
	var   inputbox=document.getElementsByName("combox_"+this.name)[0] ;

	this.value = inputbox.value;
//	alert( "aaaa this.value   "+this.value );
	this.text = inputbox.value;
//	alert( "bbbb this.text  "+this.text );
}  

combox.prototype.change=function(){  
	var   inputbox=document.getElementsByName("combox_"+this.name)[0];
	inputbox.value=this.select.options[this.select.selectedIndex].text;
	
	this.value = this.select.options[this.select.selectedIndex].value;
//	alert( "cccc this.value   "+this.value );
	this.text = this.select.options[this.select.selectedIndex].text;
//	alert( "dddd this.text  "+this.text );
	with(inputbox){
		select();
		focus();
	};
}  



function isLegalIpAddr( ipaddr )
{
	re=/^\s*(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\s*$/; //匹配IP地址的正则表达式
	if(re.test(ipaddr))
	{
		//return RegExp.$1*Math.pow(255,3))+RegExp.$2*Math.pow(255,2))+RegExp.$3*255+RegExp.$4*1
		if( RegExp.$1 > 255 || RegExp.$2 > 255 || RegExp.$3 > 255 || RegExp.$4 > 255 )
		{
			//alert("isLegalIpAddr  1111 ipaddr = " + ipaddr );
			return false;
		}
		return true;
	}
	else
	{
		//throw new Error("Not a valid IP address!")
		//alert("isLegalIpAddr  2222 ipaddr = " + ipaddr );
		return false;
	}
}

function isLegalIpRang( ipbegin, ipend )
{
	//alert( "isLegalIpRang! ipbegin="+ipbegin+"    ipend="+ipend );
	re=/^\s*(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\s*$/; //匹配IP地址的正则表达式
//	re2=/^\s*(\d+)\.(\d+)\.(\d+)\.(\d+)\s*$/g; //匹配IP地址的正则表达式
	//alert("11111111111");
	if( re.test(ipbegin) )
	{
		//alert("2222222");
		if( RegExp.$1 > 255 || RegExp.$2 > 255 || RegExp.$3 > 255 || RegExp.$4 > 255 )
			return false;		
		var beginaddr = RegExp.$1*Math.pow(256,3)+RegExp.$2*Math.pow(256,2)+RegExp.$3*256+RegExp.$4*1;
		//alert("333333");
	}
	else
	{
		//alert("444444");
		return false;	
	}
	
	if( re.test(ipend) )
	{
		//alert("5555");
		if( RegExp.$1 > 255 || RegExp.$2 > 255 || RegExp.$3 > 255 || RegExp.$4 > 255 )
			return false;		
		var endaddr = RegExp.$1*Math.pow(256,3)+RegExp.$2*Math.pow(256,2)+RegExp.$3*256+RegExp.$4*1;
		//alert("66666");
	}
	else
	{
		//alert("77777");
		return false;	
	}
	//alert( "beginaddr = "+beginaddr );
	//alert( "endaddr = "+endaddr );
	return (endaddr - beginaddr > 0)
}


function isLegalMask( mask )  
{
	re=/^\s*(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\s*$/; //匹配IP地址的正则表达式
	if(re.test(mask))
	{
		if( RegExp.$1 > 255 || RegExp.$2 > 255 || RegExp.$3 > 255 || RegExp.$4 > 255 )
			return false;
		
		var aaaa = RegExp.$1*Math.pow(256,3)+RegExp.$2*Math.pow(256,2)+RegExp.$3*256+RegExp.$4*1
		
		remask = /^1*0*$/g;
		if( aaaa.toString(2).length == 32 && remask.test( aaaa.toString(2) ) )
			return true
		
		return false;
	}
	else
	{
		//throw new Error("Not a valid IP address!")
		return false;
	}		
}



function isLegalPort( port )
{
	re = /^\s*(\d+)\s*$/;
	
	if( re.test(port) && RegExp.$1 < 65536 )
		return true;
	
	return false;
}

function isLegalRuleName( name )
{
	re = /^[\w\s-.]*[\w]+[\w\s-.]*$/;
	
	if( re.test(name) )
	{
		return true;
	}
	return false;
}	

function getPortSelectionOptions()
{
	var opts = new Array();
	
	opts[0] = new Option();
	opts[0].value='-1';
	opts[0].text='';
	opts[1] = new Option();
	opts[1].value='1';
	opts[1].text='11111';
	opts[2] = new Option();
	opts[2].value='2';
	opts[2].text='22222';
	opts[3] = new Option();
	opts[3].value='3';
	opts[3].text='33333';
	
	return opts;	
}

function PortCollector( obj, name )
{
	this.name = name;
	this.obj = obj;
	this.inputbox = null;
	this.selection = null;
	this.selected = null;
	this.opts=null;
	this.curComBoxValue="";
	this.curComBoxText="";
	this.hiddenInput="";
	this.btnAdd = null;
	this.butRemove = null;
	this.disabled = true;
	
	this.setOptionons = function( opts ){
		this.opts = opts;
		//for( var i = 0; i < this.opts.length; i++ ){
			//alert( this.opts[i].value );
		//}
	};
/*	
	this.showSelectionList = function(){
		var curFocus = document.activeElement;
		
		this.selection.focus();
		
		try{ 		
 			var WshShell = new ActiveXObject("Wscript.Shell");
 			try{
 				WshShell.SendKeys("%{DOWN}");
 			}
 			catch(e){
 				WshShell.Quit;
			}
 		}
 		catch(ex){
 			alert("当前安全级别不允许创建");
 		}

		Shell.SendKeys("%{DOWN}");
		Shell.Quit();
		curFocus.focus();
	}
	

	this.inputPropertyChange = function (){
			
	}
*/	
	this.setDisabledState = function ( state ){
		this.disabled = state;
		this.checkDisabled();
		return;
	}
	
	this.checkDisabled = function(){
		if( this.disabled == true ){
			this.inputbox.disabled = true;
			this.selection.disabled = true;
			this.selected.disabled = true;
			this.btnAdd.disabled = true;
			this.btnRemove.disabled = true;
			
			this.inputbox.style.backgroundColor="#eeeeee";
			this.selected.style.backgroundColor="#eeeeee";
		}
		else{
			this.inputbox.disabled = false;
			this.selection.disabled = false;
			this.selected.disabled = false;
			this.inputbox.style.backgroundColor="#ffffff";
			this.selected.style.backgroundColor="#ffffff";
			
			this.checkAddBtn();
			this.checkRemoveBtn();
		}
		return;
	}
	
	
	
	this.checkAddBtn = function(){
		if( this.inputbox.value != "" )
			this.btnAdd.disabled = false;
		else
			this.btnAdd.disabled = true;
	}
	
	this.checkRemoveBtn = function(){
		var options = this.selected.options;
		
		this.btnRemove.disabled = true;
		
		for( var i = 0; i < options.length; i++ ){
			if( true == options[i].selected ){
				this.btnRemove.disabled = false;
				return;
			}
		}
	}
	
	this.inputChange = function(){
		var i;
		var val = this.inputbox.value;
		
		/*
		for( i=0; i<this.opts.length; i++ ){
			if( 0 == this.opts[i].text.indexOf( val ) ){
				this.selection.selectedIndex = i;
				//this.inputbox.value = this.opts[i].text;
				this.curComBoxValue = this.opts[i].value;
				this.curComBoxText = this.opts[i].text;
				this.checkAddBtn();
				return;
			}
		}*/
		this.curComBoxValue = val;
		this.curComBoxText = val;
		this.checkAddBtn();
	
		return;
	}
	
	this.selectionChange = function(){
		//alert('1111');
		this.inputbox.value =  this.selection.options[this.selection.selectedIndex].text;	
		//alert('2222');
		this.curComBoxValue = this.selection.options[this.selection.selectedIndex].value.toString();
		//alert('3333');
		this.curComBoxText = this.selection.options[this.selection.selectedIndex].text.toString();
		this.selection.selectedIndex = 0;
		this.checkAddBtn();
	}
	
	this.resetHiddenInputValue = function(){
		var options = this.selected.options;	
		//重新根据select中的内容设置hiddenInput的值
		this.hiddenInput.value = "";
		
		if( options.length > 0 )
			this.hiddenInput.value = options[0].value;
		
		for( var i=1; i < options.length;i++){
			this.hiddenInput.value += (","+options[i].value);
		}
		return;
	}
	
	this.addPort = function(){
		if( null != this.selected ){
			var opt = new Option();
			opt.value = this.curComBoxValue;
			opt.text = this.curComBoxText;
			
			if( opt.value == "" || opt.text == "" )
				return;
				
			try
    		{
    			this.selected.add(opt,null); // standards compliant
   			}
  			catch(ex)
    		{
    			this.selected.add(opt); // IE only
    		}
    		if( "" == this.hiddenInput.value )
    			this.hiddenInput.value = opt.value;
    		else
    			this.hiddenInput.value += (","+opt.value);
    		
    		this.resetHiddenInputValue();
    		this.inputbox.value="";
    		this.curComBoxValue="";
    		this.curComBoxText="";
    		this.checkAddBtn();
		}

		return;
	}
	
	this.removePort = function(){
		var options = this.selected.options;
		
		for( var i=0; i < options.length; ){
			if( true == options[i].selected ){
				//var str = options[i].value+",|,"+options[i].value+"|"+options[i].value;
				//var rc = new RegExp( str );
				//this.hiddenInput.value = this.hiddenInput.value.toString().replace( rc,"");
				this.selected.remove(i);
			}
			else
			{//为什么在这里i++不在for里面i++?因为 如果找到有选则的项后就将其删除了，然后options.length变量，
			//每个项的index也变了，所以，如果删除了一个项后i的值就不要加，这样新的options恰好是下一个需要判断的元素。
			//否则有些项可能判断不到。
				i++;
			}
		}

		this.resetHiddenInputValue();
		this.checkRemoveBtn();
		
		return;
	}
	
	this.writeHTML = function(){
		//document.write( "<span>portCollector test</span><br />" );
		//document.write( "<Object type='application/x-oleobject' id=Shell classid='clsid:F935DC22-1CF0-11D0-ADB9-00C04FD58A0B'></Object>" );
		document.write( "<input type='hidden' name='"+name+"' id='"+name+"' value=''>" );
		this.hiddenInput = document.getElementById(name);
		document.write( "<table><tr><td>\n" );
		document.write( "<select name='selection_"+name+"' id='selection_"+name+"' style='width:100px' onchange='"+this.obj+".selectionChange();'>" )
		for( var i=0; i < this.opts.length; i++ ){
			document.write( "<option value="+this.opts[i].value+">"+this.opts[i].text+"</options>" );
		}
		document.write( "</select>" );
		this.selection = document.getElementById( "selection_"+this.name );
				
		document.write( "</td>" );
		document.write( "<td><input type='button' id='btnadd_"+this.name+"' value='add' style='width:50px' onclick='"+this.obj+".addPort();' />" );
		this.btnAdd = document.getElementById('btnadd_'+name);
		document.write( "</td>" );
		document.write( "</tr>" );
		
		document.write( "<tr>" );
		document.write( "<td>" );
		document.write( "<select name='selected_"+name+"' id='selected_"+name+"' multiple='multiple' style='width:100' onchange='"+this.obj+".checkRemoveBtn();' size=6></select>" );
		this.selected = document.getElementById( 'selected_'+name );
		document.write( "</td>");
		document.write( "<td><input type='button' value='remove' name='btnremove_"+name+"' style='width:50px' onclick='"+this.obj+".removePort();' />" );
		this.btnRemove = document.getElementById('btnremove_'+name);
		document.write( "</td></tr></table>" );
		
		document.write("<input id='input_"+this.name+"' onchange='"+this.obj+".inputChange();' onpropertychange='"+this.obj+".checkAddBtn();' " );
		document.write("style='position:absolute;width:"+(this.selection.offsetWidth-18)+";height:"+(this.selection.offsetHeight)+";left:"+getL(this.selection)+";top:"+getT(this.selection)+"'>" );		
		this.inputbox = document.getElementById('input_'+this.name);
	/*	
		//alert( this.selection );
		with(this.selection.style){ 
			left=getL(this.selection);
			top=getT(this.selection);
			position="absolute";
			clip="rect(0   "+(this.selection.offsetWidth)+"   "+this.selection.offsetHeight+"   "+(this.selection.offsetWidth-18)+")" ; 
		}
		*/
	
		this.checkAddBtn();
		this.checkRemoveBtn();
	}
	
}



function popMenuItem( show,callurl )
{
	var str;
	var len;
	this.show = show;
	this.callurl = callurl;
	
	//因为中文一个汉字在javascript中算是一个字符。这样才能得到真实的字符串长度。
	str = show;
	len = str.match(/[^ -~]/g) == null ? str.length : str.length + str.match(/[^ -~]/g).length ;
	
	this.strlen=len;
}

function popMenu( objname,z_index )
{
	this.objname=objname;
	this.menuItems = new Array();
	this.z_index = z_index;
	this.wordsize=document.body.style['fontSize'];
	this.width = 0;
	this.height = 0;
	
	if( this.wordsize == '' )
		this.wordsize = 16;//浏览器的默认字体是16号字
	
	
	this.addItem = function( menuItem ) {
		this.menuItems[this.menuItems.length] = menuItem;
		
		//alert( "show =" + menuItem.show + "  length=" + menuItem.strlen );
		this.height += this.wordsize;
		if( this.width < menuItem.strlen*this.wordsize/2 )
			this.width = menuItem.strlen*this.wordsize/2;
	}
	
	this.onMouseOver = function( obj ){
		//alert("show on");
		//obj.firstChild.style.display='block';
		document.getElementById( this.objname ).style.display='block';
	}
	
	this.onMouseOut = function( obj ) {
		//alert("show off");
		//obj.firstChild.style.display='none';
		document.getElementById( this.objname ).style.display='none';
	}
	
	this.show = function(){
		var menu_pop_out = "<div style='display:none;position:absolute; top:5px; left:0; "+";' id="+this.objname+" ><div style='width:"+(this.width+this.wordsize+2)+";height:"+this.height+";border:1px solid #666666;background-color:#f9f8f7;'>";
		for( var i=0; i<this.menuItems.length; i++ ){
			menu_pop_out += "<div style='width:"+(this.width+this.wordsize)+";height:"+this.wordsize+";padding-left:5px; padding-top:3px;' onmouseover=\"this.style.backgroundColor='#b6bdd2'\" ";
			menu_pop_out += "onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=";
			menu_pop_out += this.menuItems[i].callurl+" target=mainFrame>"+this.menuItems[i].show+"</a></div>";
		}
		menu_pop_out += "</div></div>";
	
		var menu_main = "<div style=\"position:relative; z-index:"+this.z_index+"\" onmouseover='"+this.objname+".onMouseOver(this);' onmouseout='"+this.objname+".onMouseOut(this);' >";
		menu_main += "<img src=/images/detail.gif>"+menu_pop_out+"</div>";
		
	//	alert( menu_main );
		document.write( menu_main );
	}
		
}



