function requestSettings() {
    var e = new Object;
    e = $.ajax({
        type: "GET",
        url: "/settings",
        dataType: "text",
        data: $(this).serialize(),
        success: function(e) {
            try {
                json = $.parseJSON(e), update_content()
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function update_content() {
    UpdateSheduleFields();
}

$(document).ready(function() {
    requestBoardInfo();
    requestSettings();
});


function UpdateSheduleFields()
{
    var relay = GetURLParameter('relay');
    relay -=1;//zero-based
    for(var i =0;i<14;i++)
    {
        var hour = json.Channels[relay].alarms[i].hour;
        var minute = json.Channels[relay].alarms[i].minute;
        var enabled = json.Channels[relay].alarms[i].enabled;  
        if(hour<10)
        {
            hour = zeroPad(hour);
        }
        if(minute<10)
        {
        minute = zeroPad(minute);
        }
        $("#alarm_"+(i+1)).val(hour + ":" + minute);
        $("#alam_enabeled_"+(i+1)).prop("checked", enabled)
    }
    //$("#shedule_enable").val(enabled?"1":"0");    
}


function zeroPad(numberStr) {
  return numberStr.padStart(2, "0");
}

function GetURLParameter(sParam)
{
    var sPageURL = window.location.search.substring(1);
    var sURLVariables = sPageURL.split('&');
    for (var i = 0; i < sURLVariables.length; i++)
    {
        var sParameterName = sURLVariables[i].split('=');
        if (sParameterName[0] == sParam)
        {
            return sParameterName[1];
        }
    }
}

function requestBoardInfo()
{
        $.ajax({
			type: "GET",
			url: "/boardinfo",
			dataType: "text",
			success: function(e) {
				try {
					updateBoardInfo(e)
				} catch (t) {
					console.log(t)
				}
			}
		})
}

function updateShedule()
{
    var relay = $("#Relay").val();
    
    var e = {
        alarm_time_1: $("#alarm_1").val(),
        alarm_enable_1: $("#alam_enabeled_").is(":checked");
    };
    $("#sendAuthSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/auth_settings",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                "OK" == e ? ($("#sendAuthSettingsButton").html("SAVED"), requestSettings()) : ($("#sendAuthSettingsButton").html("FAILED SAVING"), console.log("FAILED TO SAVE EMAILSETTINGS"))
            } catch (t) {
                console.log(t)
            }
        }
    })
}
