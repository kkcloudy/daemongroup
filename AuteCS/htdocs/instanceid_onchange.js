function instanceid_change( obj, href_name, un )
{
	var instanceid = obj.options[obj.selectedIndex].value;
  	var url = href_name + '?UN=' + un +'&INSTANCE_ID='+instanceid;
  	window.location.href = url;
}