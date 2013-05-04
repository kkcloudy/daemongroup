function slotid_change( obj, href_name, un )
{
	var slotid = obj.options[obj.selectedIndex].value;
	var url = href_name + '?UN=' + un +'&SLOT_ID='+slotid;
	window.location.href = url;
}
