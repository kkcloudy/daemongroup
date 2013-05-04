eagController.onUpdate = updateUI ;
eagController.onError  = handleError ;
eagClock.onTick = function () { }

function ie_getElementsByTagName(str) {
	if (str=="*") return document.all;
	else return document.all.tags(str);
}

if (document.all) 
	document.getElementsByTagName = ie_getElementsByTagName;

function hidePage(page) { 
	var e = document.getElementById(page);
	if (e != null) e.style.display='none';
}

function showPage(page) { 
	var e = document.getElementById(page);
	if (e != null) e.style.display='inline';
}

function setElementValue(elem, val, forceHTML) {
	var e = document.getElementById(elem);
	if (e != null) {
		var node = e;
		if (!forceHTML && node.firstChild) {
			node = node.firstChild;
			node.nodeValue = val;
		} else {
			node.innerHTML = val;
		}
	}
}

var has_open_originalURL = false;
var has_open_advertisingURL = false;
function updateUI (cmd ) {
	clearTimeout ( delayTimer );
		
	if ( eagController.clientState == eagController.stateCodes.NOT_AUTH ) {
		showLogonPage();
	}
	if ( eagController.clientState == eagController.stateCodes.AUTH ) {
		if ( eagController.statusURL ) {
			eagController.statusWindow = window.open(eagController.statusURL, "");
		} else {
			eagClock.start(0);
			showStatusPage();
		}
	}
    
	if ( eagController.clientState == eagController.stateCodes.AUTH_PENDING ) showWaitPage();
    
	if ( eagController.clientState == eagController.stateCodes.AUTH_FAILED ) 
   	{	
		alert( eagController.message );
		eagController.message = null;
    	eagController.clientState = eagController.stateCodes.NOT_AUTH;
   		showLogonPage();
	}
}

function handleError( code ) {
    clearTimeout(delayTimer);
    eagController.refresh();
}

/* Action triggered when buttons are pressed */
function connect() {
	var domainname = null;
    var username =  document.getElementById('username').value ;
    var password =  document.getElementById('password').value ;
    
    if (username == null || username == '')	return;

    showWaitPage();
    eagController.logon( username , password ) ;    
}

function disconnect() {
    //if (confirm("Are you sure you want to disconnect now?")) {
	eagClock.stop();
	showWaitPage(1000);
	eagController.logoff();
   // }    
    
    return false;
}

/* User interface pages update */
function showLogonPage() {
	has_open_originalURL = false;
    showPage("logonPage");
    hidePage("statusPage");
    hidePage("waitPage");
    hidePage("errorPage");
}

function showErrorPage() {
    showPage("errorPage");
    hidePage("logonPage");
    hidePage("statusPage");
    hidePage("waitPage");
    
    //if(eagController.message == null || eagController.message == '')
    //{
    //	eagController.message = "帐号密码错误或用户不存在";
    //}
    
    //setElementValue("errorMessage",eagController.message);
    //eagController.message = null;
}

function showStatusPage() {
    hidePage("logonPage");
    showPage("statusPage");
    hidePage("waitPage");
    hidePage("errorPage");
    
    var msg = "";
    if ( eagController.message != null && eagController.message != "") { 
		//setElementValue("statusMessage", eagController.message, true);
		msg = eagController.message;
		eagController.message = null;
		alert(msg);
    }

	setElementValue("sessionId",
		eagController.session.sessionId ?
		eagController.session.sessionId :
		 "Not available");

	setElementValue("sessionTimeout",
		eagController.formatTime(eagController.session.sessionTimeout, 'unlimited'));

    setElementValue("idleTimeout",
		eagController.formatTime(eagController.session.idleTimeout, 'unlimited'));
		    
    setElementValue("startTime",
		eagController.session.startTime ?
		eagController.session.startTime :
		"Not available");
   		    
    setElementValue("sessionTime",
		eagController.formatTime(eagController.accounting.sessionTime));
		
	if ( eagController.redir && eagController.redir.originalURL != null && eagController.redir.originalURL != '') {
			setElementValue('originalURL', '<a target="_blank" href="'+eagController.redir.originalURL+
				'">'+eagController.redir.originalURL+'</a>', true);
			if (!has_open_originalURL)
			{
				window.open(eagController.redir.originalURL);
				has_open_originalURL = true;
			}
	}
	else
	{
		setElementValue("originalURL", "Not available", true);
	}
	
    if (	eagController.redir && eagController.redir.advertisingURL != null 
		&&	eagController.redir.advertisingURL != '') {
                setElementValue('advertisingURL', '<a target="_blank" href="'+eagController.redir.advertisingURL+
                        '">'+eagController.redir.advertisingURL+'</a>', true);
                if (!has_open_advertisingURL)
                {
                        window.open(eagController.redir.advertisingURL);
                        has_open_advertisingURL = true;
                }
	}
	else
	{
	        setElementValue("advertisingURL", "Not available", true);
	}

	eagClock.resync (eagController.accounting.sessionTime);
}

function showWaitPage(delay) {
    /* Wait for delay  */
    clearTimeout(delayTimer);	
    if (typeof(delay) == 'number' && (delay > 10)) {
		delayTimer= setTimeout('showWaitPage(0)' , delay);
		return;
	}
    
    /* show the waitPage */
    hidePage("logonPage");
    hidePage("statusPage");
    showPage("waitPage");
    hidePage("errorPage");
}


function getnasip()
{
	var curhref = window.location.href;
	var index1=curhref.indexOf("\/\/");
	var index2=curhref.indexOf("\/portal");
	if( index1 >= index2 )
	{
		alert("获得nasip错误!");	
		return;
	}
	return curhref.substring(index1+2,index2);
}

var delayTimer; // global reference to delayTimer
window.onload = function() {
    var logonForm = document.getElementById('logonForm');

    var head = document.getElementsByTagName("head")[0];
    if (head == null) head = document.body;

    if (logonForm == null) {
		logonForm = document.getElementById('loginForm');
    }
	
    if (logonForm == null) {
		try {
			logonForm = document.createElement('div');
			logonForm.setAttribute('id', 'logonForm');
			logonForm.setAttribute('name', 'logonForm');
			var thisScript = document.getElementById('eagjs');
			if (thisScript != null) {
				thisScript.parentNode.insertBefore(logonForm, thisScript);
			} else {
				document.body.appendChild(logonForm);
			}
		} catch(exception) {
			document.body.innerHTML += "<div id='logonForm'></div>";
        }
        logonForm = document.getElementById('logonForm');
    }

    if (logonForm.innerHTML == '') {
		logonForm.innerHTML='Error loading generic login form';
    }

	var jsonjs="/cgi-bin/wp_eagcustomer.cgi?nasip="+getnasip();
	
	var script = document.getElementById('jsonjsobj');
	if (script == null) {
		script = document.createElement('script');
	    script.id = 'jsonjsobj';
	    script.type = 'text/javascript';
	    script.src = jsonjs;
	    var head = document.getElementsByTagName("head")[0];
	    if (head == null) head = document.body;
	    head.appendChild(script);
	}
}

