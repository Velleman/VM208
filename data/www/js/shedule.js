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
    });

    e = $.ajax({
        type: "GET",
        url: "/getalarms",
        dataType: "text",
        data: {
            channel:1
        },
        success: function(e) {
            try {
                json = $.parseJSON(e);
                console.log(json);
            } catch (t) {
                console.log(t)
            }
        }
    });
}

function update_content() {
    var relay = GetURLParameter('relay');
    $("#Relay").val(relay);
    UpdateSheduleFields(relay);
}

$(document).ready(function() {
    requestBoardInfo();
    requestSettings();
    $("#Relay").change(function(){
        var relay = $("#Relay").val(); 
        UpdateSheduleFields(relay);
    });
});

function ValidateShedule()
{
    var valid = false;
    var field = 1;
    for(var i =0;i<7;i++)
    {
        if(($("#alarm_"+field).val() == $("#alarm_"+(field+1)).val()) && $("#alam_enabeled_"+field).is(":checked") && $("#alam_enabeled_"+(field+1)).is(":checked"))
        {
            return false;
        }
    }
    return true;    
}

function UpdateSheduleFields(relay)
{
    
    
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
  return String(numberStr).padStart(2, "0");
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
    if(ValidateShedule())
    {
    var relay = $("#Relay").val();
    
    var e = {
        relay: relay,        
        alarm_time_1: $("#alarm_1").val(),
        alarm_enable_1: $("#alam_enabeled_1").is(":checked"),
        alarm_time_2: $("#alarm_2").val(),
        alarm_enable_2: $("#alam_enabeled_2").is(":checked"),
        alarm_time_3: $("#alarm_3").val(),
        alarm_enable_3: $("#alam_enabeled_3").is(":checked"),
        alarm_time_4: $("#alarm_4").val(),
        alarm_enable_4: $("#alam_enabeled_4").is(":checked"),
        alarm_time_5: $("#alarm_5").val(),
        alarm_enable_5: $("#alam_enabeled_5").is(":checked"),
        alarm_time_6: $("#alarm_6").val(),
        alarm_enable_6: $("#alam_enabeled_6").is(":checked"),
        alarm_time_7: $("#alarm_7").val(),
        alarm_enable_7: $("#alam_enabeled_7").is(":checked"),
        alarm_time_8: $("#alarm_8").val(),
        alarm_enable_8: $("#alam_enabeled_8").is(":checked"),
        alarm_time_9: $("#alarm_9").val(),
        alarm_enable_9: $("#alam_enabeled_9").is(":checked"),
        alarm_time_10: $("#alarm_10").val(),
        alarm_enable_10: $("#alam_enabeled_10").is(":checked"),
        alarm_time_11: $("#alarm_11").val(),
        alarm_enable_11: $("#alam_enabeled_11").is(":checked"),
        alarm_time_12: $("#alarm_12").val(),
        alarm_enable_12: $("#alam_enabeled_12").is(":checked"),
        alarm_time_13: $("#alarm_13").val(),
        alarm_enable_13: $("#alam_enabeled_13").is(":checked"),
        alarm_time_14: $("#alarm_14").val(),
        alarm_enable_14: $("#alam_enabeled_14").is(":checked"),
        
    };
    $("#sendAuthSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/shedule_set",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                json = $.parseJSON(e);
                var relay = $("#Relay").val(); 
                UpdateSheduleFields(relay);
            } catch (t) {
                console.log(t)
            }
        }
    });
    }
    else{
        alert("Shedule form is incorrect");
    }
}

function updateBoardInfo(e)
{
        var boardInfo = $.parseJSON(e);
        $("#boardname").text(boardInfo.name);
        $("#mac_eth").text(boardInfo.MAC_ETH);
        $("#mac_wifi").text(boardInfo.MAC_WIFI);
        var uptime = boardInfo.time;
        uptime = Math.floor(uptime/1000000);
        var years = Math.floor(uptime/31556952);
        uptime -= (years * 31556952);
        var days = Math.floor(uptime / 86400);
        uptime -= (days * 86400);
        var hours = Math.floor(uptime /3600);
        uptime -= hours * 3600;
        var minutes = Math.floor(uptime /60);
        uptime -= minutes * 60;
        var seconds = Math.floor(uptime);
        var time = years + "y " + days + "d " + hours + "h " + minutes + "m " + seconds + "s";
        $("#uptime").text(time);
        $("#version").text(boardInfo.firmware);
        $("#localtime").text(boardInfo.LOCAL_TIME);
}

