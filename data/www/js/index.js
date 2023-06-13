var selectedInterface = -1;
var selectedSocket = 0;
var jsonLayout;
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
    e = $.ajax({
        type: "GET",
        url: "/layout",
        dataType: "text",
        data: $(this).serialize(),
        success: function (e) {
            try {
                json = $.parseJSON(e);
                applyModuleLayout(json);
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function applyModuleLayout(json) {
    //populate dropdowns
    jsonLayout = json;
    if (json.VM208EX) {
        SelectInterface.innerHTML += "<option value=\"0\">Extention</option>";
    } else {
        if (json.Interfaces.length) {
            for (var i = 0; i < json.Interfaces.length; i++) {
                var name = i + 1;
                SelectInterface.innerHTML += "<option value=\"" + name + "\">Interface " + name + "</option>";
            }
        } else {

        }
    }

}

function isElementVisible(e) {
    var t = $(window).scrollTop(),
        a = $(e).offset().top,
        n = a + $(e).height();
    return n >= t
}

function update_content() {
    update_names()
}

function requestShedule(channel) {

    var relay = converToChannelID(-1, 0, channel);
    relay += 1;
    console.log("ChannelID is:" + relay);
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
                update_sheduler_state(json, relay);
                channel++;
                if (jsonLayout.VM208EX == true) {
                    if (channel < 12)
                        requestShedule(channel);
                }else{
                    if(channel < 4)
                        requestShedule(channel);
                }
            } catch (t) {
                console.log(t)
            }
        }
    });

}

function update_sheduler_state(shedule, relay) {
    for (var j = 0; j < 14; j++) {
        if (shedule.Channel.alarms[j].enabled) {
            $("#dot" + relay).css("background-color", "#3f9f31");
        }
    }
}

function update_names() {
    //var e = $("#name_card").val(json.BOARDNAME);
    /*for (var i = 1; i <= 12; i++) {
        $("#Name" + i).html(json.Channels[i - 1].name);
    }
    $("#i1Name").html(json.NAME_INPUT);
    $("#m1Name").html(json.NAME_MOSFET1);
    $("#m2Name").html(json.NAME_MOSFET2);*/
}




function sendRelay(e, t) {
    var relayName = t.id;
    relayName = relayName.replace('relay', '');
    relayName = relayName.replace('Status', '');
    var payload = {
        index: parseInt(relayName),
        state: "TURN OFF" == $(t).html() ? "0" : "1",
        interface: selectedInterface,
        socket: selectedSocket
    };
    $.ajax({
        type: "POST",
        url: "/relay",
        dataType: "text",
        data: payload,
        success: function (e) {
            try {
                updateIO(e)
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function sendMosfet(e, t) {
    relaysettings = "TURN OFF" == $(t).html() ? "/" + e + "/off" : "/" + e + "/on";
    var payload = {
        index: e,
        state: "TURN OFF" == $(t).html() ? "0" : "1"
    };
    $.ajax({
        type: "POST",
        url: "/mosfet",
        dataType: "text",
        data: payload,
        success: function (e) {
            try {
                updateIO(e)
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function sendPulse(i, t) {
    if (t.innerHTML == "START") {
        var relayName = t.id;
        relayName = relayName.replace('pulse', '');
        relayName = relayName.replace('Start', '');
        var payload = {
            index: parseInt(relayName),
            value: $("#value_pulse" + relayName).val(),
            interface: selectedInterface,
            socket: selectedSocket
        };
        t.innerHTML = "STOP";
        $.ajax({
            type: "POST",
            url: "pulse",
            dataType: "text",
            data: payload,
            success: function (e) {
                try {
                    //updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
    } else {
        var relayName = t.id;
        relayName = relayName.replace('pulse', '');
        relayName = relayName.replace('Start', '');
        var payload = {
            index: parseInt(relayName),
            interface: selectedInterface,
            socket: selectedSocket
        };
        t.innerHTML = "START";
        $.ajax({
            type: "POST",
            url: "stoppulse",
            dataType: "text",
            data: payload,
            success: function (e) {
                try {
                    //updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
    }
}

function sendTimer(i, t) {
    if (t.innerHTML == "START") {
        var relayName = t.id;
        relayName = relayName.replace('timer', '');
        relayName = relayName.replace('Start', '');
        var payload = {
            index: parseInt(relayName),
            value: $("#value_timer" + relayName).val(),
            interface: selectedInterface,
            socket: selectedSocket
        };
        t.innerHTML = "STOP";
        $.ajax({
            type: "POST",
            url: "timer",
            dataType: "text",
            data: payload,
            success: function (e) {
                try {
                    updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
    }
    else {
        var relayName = t.id;
        relayName = relayName.replace('timer', '');
        relayName = relayName.replace('Start', '');
        var payload = {
            index: parseInt(relayName),
            interface: selectedInterface,
            socket: selectedSocket
        };
        t.innerHTML = "START";
        $.ajax({
            type: "POST",
            url: "stoptimer",
            dataType: "text",
            data: payload,
            success: function (e) {
                try {
                    updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
    }
}

function timerClearEvent() {
    $("#statuslabel").text("")
}


function sendRequestBootloader() {
    $.ajax({
        type: "GET",
        url: "/bootloader",
        dataType: "text",
        success: function () { }
    })
}






function sendWifiNetworkSettings() {
    if (ValidateIPaddress($("#value_wifi_ipaddress").val()) && ValidateIPaddress($("#value_wifi_gateway").val()) && ValidateIPaddress($("#value_wifi_subnetmask").val()) && ValidateIPaddress($("#value_wifi_primarydns").val()) && ValidateIPaddress($("#value_wifi_secondarydns").val())) {

        var e = {
            dhcpenable: $("#dhcp_enabled_wifi_checkbox").is(":checked") ? true : false,
            ipaddress: $("#value_wifi_ipaddress").val(),
            gateway: $("#value_wifi_gateway").val(),
            subnetmask: $("#value_wifi_subnetmask").val(),
            primarydns: $("#value_wifi_primarydns").val(),
            secondarydns: $("#value_wifi_secondarydns").val()
        };

        $("#sendNetworkSettingsButton").html("SAVING..."); {
            $.ajax({
                type: "POST",
                url: "/wifi_ip_save",
                dataType: "text",
                data: e,
                success: function (e) {
                    try {
                        "OK" == e ? ($("#sendNetworkSettingsButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE NETWORKSETTINGS")
                    } catch (t) {
                        console.log(t)
                    }
                }
            })
        }
    } else $("#sendNetworkSettingsButton").html("INVALID IP ADDRESSES")
}

function sliderOnInitialized() {
    $("body").css("display", "block")
}

function timerRelayEvent() {

    if (location.pathname == "/index.html" || location.pathname == "/") {
        var payload = {
            interface: selectedInterface,
            socket: selectedSocket
        };
        $.ajax({
            type: "GET",
            url: "/status",
            dataType: "text",
            data: payload,
            success: function (e) {
                try {
                    updateIO(e), setTimeout(function () {
                        timerRelayEvent()
                    }, 500)
                } catch (t) {
                    console.log(t)
                }
            },
            error: function (e) {
                setTimeout(function () {
                    timerRelayEvent()
                }, 500)
            }
        })
    }
}


function updateIO(e) {
    var t = $.parseJSON(e);
    try {

        //var a;
        if (selectedInterface == "-1") {
            for (a = 1; a <= 4; a++) {
                $("#relay" + a + "Status").html(t.Channels[a - 1].state ? "TURN OFF" : "TURN ON");
                $("#pulse" + a + "Start").html(t.Channels[a - 1].pulseActive ? "STOP" : "START");
                $("#timer" + a + "Start").html(t.Channels[a - 1].timerActive ? "STOP" : "START");
                $("#Name" + a).html(t.Channels[a - 1].name);
            }
        } else {
            for (a = 1; a <= 8; a++) {
                $("#relay" + a + "Status").html(t.Channels[a - 1].state ? "TURN OFF" : "TURN ON");
                $("#pulse" + a + "Start").html(t.Channels[a - 1].pulseActive ? "STOP" : "START");
                $("#timer" + a + "Start").html(t.Channels[a - 1].timerActive ? "STOP" : "START");
                $("#Name" + a).html(t.Channels[a - 1].name);
            }
        }

    } catch (s) {
        console.log(s)
    }
    $("#mosfet1Status").html(t.m1 ? "TURN OFF" : "TURN ON");
    $("#mosfet2Status").html(t.m2 ? "TURN OFF" : "TURN ON");
    $("#inputStatus").html(t.input ? "ON" : "OFF");
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

function openSheduler(e, f) {
    id = f.id;
    id = id.replace('shedule', '');
    id = id.replace('Start', '');
    location.href = "shedule.html?relay=" + id + "&interface=" + selectedInterface + "&socket=" + selectedSocket;
}

function enableButtons() {
    var e;
    if (selectedInterface == "-1")
        totalChannels = 4;
    else
        totalChannels = 8;
    for (e = 1; e <= totalChannels; e++) {
        $("#relay" + e + "Status").removeClass("pure-button-disabled");
        $("#relay" + e + "Status").click(function () {
            sendRelay(e, this);
        });
        $("#pulse" + e + "Start").removeClass("pure-button-disabled");
        $("#pulse" + e + "Start").click(function () {
            sendPulse(e, this);
        });
        $("#timer" + e + "Start").removeClass("pure-button-disabled");
        $("#timer" + e + "Start").click(function () {
            sendTimer(e, this);
        });
        $("#shedule" + e + "Start").click(function () {
            openSheduler(e, this);
        });

    }
}
var json, notif_select = new Object;
$(document).ready(function () {
    requestBoardInfo(), requestSettings(), getStatusForTable(), $("#splashscreen").delay(750).fadeOut(500);
    $('#SelectInterface').on('change', function () {
        selectedInterface = this.value;
        updateSocket();
        generateTable();
    });
    $('#SelectSocket').on('change', function () {
        selectedSocket = this.value;
        generateTable();
    });

});

function updateSocket() {

    SelectSocket.innerHTML = "";
    if (selectedInterface == "0" || selectedInterface == "-1") {
        SelectSocket.innerHTML = "<option value=\"0\">None</option>";
        selectedSocket = 0;
    } else {
        selectedSocket = 0;
        if (jsonLayout.Interfaces[selectedInterface - 1].Socket1) {
            SelectSocket.innerHTML += "<option value=\"1\">1</option>";
            if (!selectedSocket)
                selectedSocket = 1;
        }
        if (jsonLayout.Interfaces[selectedInterface - 1].Socket2) {
            SelectSocket.innerHTML += "<option value=\"2\">2</option>";
            if (!selectedSocket)
                selectedSocket = 2;
        }
        if (jsonLayout.Interfaces[selectedInterface - 1].Socket3) {
            SelectSocket.innerHTML += "<option value=\"3\">3</option>";
            if (!selectedSocket)
                selectedSocket = 3;
        }
        if (jsonLayout.Interfaces[selectedInterface - 1].Socket4) {
            SelectSocket.innerHTML += "<option value=\"4\">4</option>";
            if (!selectedSocket)
                selectedSocket = 4;
        }
    }
}
var current_slide = 0;
var totalChannels;
function getStatusForTable() {
    var e = {
        interface: selectedInterface,
        socket: selectedSocket
    };
    $.ajax({
        type: "GET",
        url: "/status",
        dataType: "text",
        data: e,
        success: function (e) {
            try {
                generateTable(e);
                requestShedule(0);
            } catch (t) {
                console.log(t)
            }
        },
    });


}

function generateTable(e) {
    var channelId = 1;
    var ioData = $.parseJSON(e);
    var table = $('#VM208TableBody');
    table.empty();
    var rows;
    if (selectedInterface == "-1")
        rows = 5;
    else
        rows = 9;
    for (i = 1; i < rows; i++) {
        table.append('<tr> \
        <td id="Name'+ channelId + '"> RELAY' + i + ' </td> \
        <td> <button class="pure-button relayButton pure-button-disabled" id=relay'+ channelId + 'Status>OFF</button> </td> \
        <td class="col3" > <input type="number" style="width:30%" name="value_pulse'+ channelId + '" id="value_pulse' + channelId + '" value="100" min="1" max="60000"><label for="value_pulse' + channelId + '">ms</label>  </label> <button class="pure-button pure-button-disabled" id=pulse' + channelId + 'Start>START</button></form> </td> \
        <td class="col4" > <input type="number" style="width:30%" name="value_timer'+ channelId + '" id="value_timer' + channelId + '" value="1" min="1" max="60000"><label for="value_timer' + channelId + '">min</label> <button class="pure-button pure-button-disabled" id=timer' + channelId + 'Start>START</button> </value> </td> \
        <td  class="col5" > <button class="pure-button" id="shedule'+ channelId + 'Start">EDIT</button> <span class="dot" id="dot' + channelId + '"></span> </td> \
     </tr>');

        $("#relay" + channelId + "Status").removeClass("pure-button-disabled");
        $("#relay" + channelId + "Status").click(function () {
            sendRelay(channelId, this);
        });
        $("#pulse" + channelId + "Start").removeClass("pure-button-disabled");
        $("#pulse" + channelId + "Start").click(function () {
            sendPulse(channelId, this);
        });
        $("#timer" + channelId + "Start").removeClass("pure-button-disabled");
        $("#timer" + channelId + "Start").click(function () {
            sendTimer(channelId, this);
        });
        $("#shedule" + channelId + "Start").removeClass("pure-button-disabled");
        $("#shedule" + channelId + "Start").click(function () {
            openSheduler(channelId, this);
        });
        channelId++;
    }

    $("#mosfet1Status").removeClass("pure-button-disabled");
    $("#mosfet1Status").click(function () {
        sendMosfet(1, this);
    });
    $("#mosfet2Status").removeClass("pure-button-disabled");
    $("#mosfet2Status").click(function () {
        sendMosfet(2, this);
    });


}


$(function () {
    timerRelayEvent();
});


function converToChannelID(interface, socket, relay) {
    var channelID = 0;
    if (interface == -1) {
        channelID = relay;
    }
    else if (interface == 0) {
        channelID = parseInt(relay) + 4;
    }
    else {
        channelID = 12 + ((interface - 1) * 32) + ((socket - 1) * 8) + relay;
    }
    return channelID;
}