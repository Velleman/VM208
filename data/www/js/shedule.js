function requestSettings() {
    var e = new Object;
    e = $.ajax({
        type: "GET",
        url: "/settings",
        dataType: "text",
        data: $(this).serialize(),
        success: function (e) {
            try {
                json = $.parseJSON(e), update_content()
            } catch (t) {
                console.log(t)
            }
        }
    });
    var relay = GetURLParameter('relay');
    
    e = $.ajax({
        type: "GET",
        url: "/getalarms",
        dataType: "text",
        data: {
            channel: relay
        },
        success: function (e) {
            try {
                json = $.parseJSON(e);
                console.log(json);
                applyShedule(json);
                
            } catch (t) {
                console.log(t)
            }
        }
    });

    e = $.ajax({
        type: "GET",
        url: "/status",
        dataType: "text",
        success: function (e) {
            try {
                json = $.parseJSON(e);
                console.log(json);
                populateDropDown(json);
                $("#Relay").val(relay); //Set relay as selected option
            } catch (t) {
                console.log(t)
            }
        }
    });
}

function populateDropDown(json) {
    var dropdown = $("#Relay");
    var index = 1;
    for (var i = 0; i < 4; i++) {
        console.log(json.Interface0.VM208[i].name);
        dropdown[0].innerHTML += "<option value="+index+">"+json.Interface0.VM208[i].name +"</option>";
        index++;
    }
    try {
        console.log(json.Interface1.VM208EX[0].name);
        dropdown[0].innerHTML += "<option value="+index+">"+json.Interfaces[i][j][k].name +"</option>";
    }
    catch {
        index+=8;
        for (var i = 0; i < json.Interfaces.length; i++)//Interfaces
        {
            for (var j = 0; j < json.Interfaces[i].length; j++)//Modules
            {
                for (var k = 0; k < json.Interfaces[i][j].length; k++)//Channels
                {
                    dropdown[0].innerHTML += "<option value="+index+">"+json.Interfaces[i][j][k].name +"</option>";
                    index++;
                }
            }
        }
    }
}

function applyShedule(json) {
    for (var i = 0; i < 14; i++) {
        var hour = json.Channel.alarms[i].hour;
        var minute = json.Channel.alarms[i].minute;
        var enabled = json.Channel.alarms[i].enabled;
        if (hour < 10) {
            hour = zeroPad(hour);
        }
        if (minute < 10) {
            minute = zeroPad(minute);
        }
        if (i == 0) {
            $("#alarm_7").val(hour + ":" + minute);
            $("#alam_enabeled_7").prop("checked", enabled)
        } else if (i == 7) {
            $("#alarm_14").val(hour + ":" + minute);
            $("#alam_enabeled_14").prop("checked", enabled)
        } else {
            $("#alarm_" + (i)).val(hour + ":" + minute);
            $("#alam_enabeled_" + (i)).prop("checked", enabled)
        }

    }
}

function update_content() {
    var relay = GetURLParameter('relay');
    e = $.ajax({
        type: "GET",
        url: "/getalarms",
        dataType: "text",
        data: {
            channel: relay
        },
        success: function (e) {
            try {
                json = $.parseJSON(e);
                console.log(json);
                applyShedule(json);
            } catch (t) {
                console.log(t)
            }
        }
    });
}

$(document).ready(function () {
    requestBoardInfo();
    requestSettings();
    $("#Relay").change(function () {
        var relay = $("#Relay").val();
        e = $.ajax({
            type: "GET",
            url: "/getalarms",
            dataType: "text",
            data: {
                channel: relay
            },
            success: function (e) {
                try {
                    json = $.parseJSON(e);
                    applyShedule(json);
                } catch (t) {
                    console.log(t)
                }
            }
        });
    });
});

function ValidateShedule() {
    var valid = false;
    var field = 1;
    for (var i = 0; i < 7; i++) {
        if (($("#alarm_" + field).val() == $("#alarm_" + (field + 1)).val()) && $("#alam_enabeled_" + field).is(":checked") && $("#alam_enabeled_" + (field + 1)).is(":checked")) {
            return false;
        }
    }
    return true;
}



function zeroPad(numberStr) {
    return String(numberStr).padStart(2, "0");
}

function GetURLParameter(sParam) {
    var sPageURL = window.location.search.substring(1);
    var sURLVariables = sPageURL.split('&');
    for (var i = 0; i < sURLVariables.length; i++) {
        var sParameterName = sURLVariables[i].split('=');
        if (sParameterName[0] == sParam) {
            return sParameterName[1];
        }
    }
}

function requestBoardInfo() {
    $.ajax({
        type: "GET",
        url: "/boardinfo",
        dataType: "text",
        success: function (e) {
            try {
                updateBoardInfo(e)
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function updateShedule() {
    if (ValidateShedule()) {
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
        $("#sendAuthSettingsButton").innerHTML = "SAVING...";
        $.ajax({
            type: "POST",
            url: "/shedule_set",
            dataType: "text",
            data: e,
            success: function (e) {
                try {
                    $("#sendAuthSettingsButton").innerHTML = "Save"
                    /*json = $.parseJSON(e);
                    var relay = $("#Relay").val();
                    applyShedule(json);*/
                } catch (t) {
                    console.log(t)
                }
            }
        });
    }
    else {
        alert("Shedule form is incorrect");
    }
}

function updateBoardInfo(e) {
    var boardInfo = $.parseJSON(e);
    $("#boardname").text(boardInfo.name);
    $("#mac_eth").text(boardInfo.MAC_ETH);
    $("#mac_wifi").text(boardInfo.MAC_WIFI);
    var uptime = boardInfo.time;
    uptime = Math.floor(uptime / 1000000);
    var years = Math.floor(uptime / 31556952);
    uptime -= (years * 31556952);
    var days = Math.floor(uptime / 86400);
    uptime -= (days * 86400);
    var hours = Math.floor(uptime / 3600);
    uptime -= hours * 3600;
    var minutes = Math.floor(uptime / 60);
    uptime -= minutes * 60;
    var seconds = Math.floor(uptime);
    var time = years + "y " + days + "d " + hours + "h " + minutes + "m " + seconds + "s";
    $("#uptime").text(time);
    $("#version").text(boardInfo.firmware);
    $("#localtime").text(boardInfo.LOCAL_TIME);
}

