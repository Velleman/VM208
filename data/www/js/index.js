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

function isElementVisible(e) {
    var t = $(window).scrollTop(),
        a = $(e).offset().top,
        n = a + $(e).height();
    return n >= t
}

function update_content() {
    update_names()
}

function update_names() {
    //var e = $("#name_card").val(json.BOARDNAME);
    for(var i =1;i<=12;i++)
    {
        $("#Name"+i).html(json.Channels[i-1].name);
    }   
    $("#i1Name").html(json.NAME_INPUT);
    $("#m1Name").html(json.NAME_MOSFET1);
    $("#m2Name").html(json.NAME_MOSFET2);
}




function sendRelay(e, t) {
    relaysettings = "TURN OFF" == $(t).html() ? "/" + e + "/off" : "/" + e + "/on";
    var payload = {
        index: e,
        state: "TURN OFF" == $(t).html() ? "0" : "1"
    };
    $.ajax({
        type: "POST",
        url: "/relay",
        dataType: "text",
        data: payload,
        success: function(e) {
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
        success: function(e) {
            try {
                updateIO(e)
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function sendPulse(i,t) {

        $.ajax({
            type: "POST",
            url: "pulse",
            dataType: "text",
            data: {index : i,time:t},
            success: function(e) {
                try {
                    updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
}

function sendTimer(i,t) {

        $.ajax({
            type: "POST",
            url: "timer",
            dataType: "text",
            data: {index : i,time:t},
            success: function(e) {
                try {
                    updateIO(e)
                } catch (t) {
                    console.log(t)
                }
            }
        })
}

function timerClearEvent() {
    $("#statuslabel").text("")
}


function sendRequestBootloader() {
    $.ajax({
        type: "GET",
        url: "/bootloader",
        dataType: "text",
        success: function() {}
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
					success: function(e) {
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

	if(location.pathname == "/index.html" || location.pathname =="/")
	{
		$.ajax({
			type: "GET",
			url: "/status",
			dataType: "text",
			success: function(e) {
				try {
					updateIO(e), setTimeout(function() {
						timerRelayEvent()
					}, 500)
				} catch (t) {
					console.log(t)
				}
			},
            error: function(e){
                setTimeout(function() {
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
        //for (a = 1; 4 >= a; a++) $("#relay" + a + "Status").html(t.relays[a - 1] ? "ON" : "OFF");
        $("#relay1Status").html(t.relay1 ? "TURN OFF" : "TURN ON");
        $("#relay2Status").html(t.relay2 ? "TURN OFF" : "TURN ON");
        $("#relay3Status").html(t.relay3 ? "TURN OFF" : "TURN ON");
        $("#relay4Status").html(t.relay4 ? "TURN OFF" : "TURN ON");
        $("#relay5Status").html(t.relay5 ? "TURN OFF" : "TURN ON");
        $("#relay6Status").html(t.relay6 ? "TURN OFF" : "TURN ON");
        $("#relay7Status").html(t.relay7 ? "TURN OFF" : "TURN ON");
        $("#relay8Status").html(t.relay8 ? "TURN OFF" : "TURN ON");
        $("#relay9Status").html(t.relay9 ? "TURN OFF" : "TURN ON");
        $("#relay10Status").html(t.relay10 ? "TURN OFF" : "TURN ON");
        $("#relay11Status").html(t.relay11 ? "TURN OFF" : "TURN ON");
        $("#relay12Status").html(t.relay12 ? "TURN OFF" : "TURN ON");
        t.isExtConnected ?  $("#extRelay").show():$("#extRelay").hide();
        $("#mosfet1Status").html(t.mosfet1 ? "TURN OFF" : "TURN ON");
        $("#mosfet2Status").html(t.mosfet2 ? "TURN OFF" : "TURN ON");
        $("#inputStatus").html(t.input ? "CLOSED" : "OPEN");
    } catch (s) {
        console.log(s)
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

function enableButtons() {
    var e;
    for (e = 1; 12 >= e; e++) $("#relay" + e + "Status").removeClass("pure-button-disabled"), $("#mosfet" + e + "Status").removeClass("pure-button-disabled"), $("#pulse" + e + "Start").removeClass("pure-button-disabled"), $("#timer" + e + "Start").removeClass("pure-button-disabled")
}
var json, notif_select = new Object;
$(document).ready(function() {
    requestBoardInfo(),requestSettings(), $("#splashscreen").delay(750).fadeOut(500), enableButtons()});
var current_slide = 0;
$(function() {
    timerRelayEvent(), 
        $("#relay1Status").click(function() {
        sendRelay(1, this)
    }), $("#relay2Status").click(function() {
        sendRelay(2, this)
    }), $("#relay3Status").click(function() {
        sendRelay(3, this)
    }), $("#relay4Status").click(function() {
        sendRelay(4, this)
    }), $("#relay5Status").click(function() {
        sendRelay(5, this)
    }), $("#relay6Status").click(function() {
        sendRelay(6, this)
    }), $("#relay7Status").click(function() {
        sendRelay(7, this)
    }), $("#relay8Status").click(function() {
        sendRelay(8, this)
    }), $("#relay9Status").click(function() {
        sendRelay(9, this)
    }), $("#relay10Status").click(function() {
        sendRelay(10, this)
    }), $("#relay11Status").click(function() {
        sendRelay(11, this)
    }), $("#relay12Status").click(function() {
        sendRelay(12, this)
    }), $("#mosfet1Status").click(function() {
        sendMosfet(1, this)
    }), $("#mosfet2Status").click(function() {
        sendMosfet(2, this)
    }), $("#pulse1Start").click(function() {
        sendPulse(1,$("#value_pulse1").val(), this)
    }), $("#pulse2Start").click(function() {
        sendPulse(2,$("#value_pulse2").val(), this)
    }), $("#pulse3Start").click(function() {
        sendPulse(3,$("#value_pulse3").val(), this)
    }), $("#pulse4Start").click(function() {
        sendPulse(4,$("#value_pulse4").val(), this)
    }), $("#pulse5Start").click(function() {
        sendPulse(5,$("#value_pulse5").val(), this)
    }), $("#pulse6Start").click(function() {
        sendPulse(6,$("#value_pulse6").val(), this)
    }), $("#pulse7Start").click(function() {
        sendPulse(7,$("#value_pulse7").val(), this)
    }), $("#pulse8Start").click(function() {
        sendPulse(8,$("#value_pulse8").val(), this)
    }), $("#pulse9Start").click(function() {
        sendPulse(9,$("#value_pulse9").val(), this)
    }), $("#pulse10Start").click(function() {
        sendPulse(10,$("#value_pulse10").val(), this)
    }), $("#pulse11Start").click(function() {
        sendPulse(11,$("#value_pulse11").val(), this)
    }), $("#pulse12Start").click(function() {
        sendPulse(12,$("#value_pulse12").val(), this)
    }), $("#timer1Start").click(function() {
        sendTimer(1,$("#value_timer1").val(), this)
    }), $("#timer2Start").click(function() {
        sendTimer(2,$("#value_timer2").val(), this)
    }), $("#timer3Start").click(function() {
        sendTimer(3,$("#value_timer3").val(), this)
    }), $("#timer4Start").click(function() {
        sendTimer(4,$("#value_timer4").val(), this)
    }), $("#timer5Start").click(function() {
        sendTimer(5,$("#value_timer5").val(), this)
    }), $("#timer6Start").click(function() {
        sendTimer(6,$("#value_timer6").val(), this)
    }), $("#timer7Start").click(function() {
        sendTimer(7,$("#value_timer7").val(), this)
    }), $("#timer8Start").click(function() {
        sendTimer(8,$("#value_timer8").val(), this)
    }), $("#timer9Start").click(function() {
        sendTimer(9,$("#value_timer9").val(), this)
    }), $("#timer10Start").click(function() {
        sendTimer(10,$("#value_timer10").val(), this)
    }), $("#timer11Start").click(function() {
        sendTimer(11,$("#value_timer11").val(), this)
    }), $("#timer12Start").click(function() {
        sendTimer(12,$("#value_timer12").val(), this)
    }),
    $("#shedule1Start").click(function(){
        location.href = "shedule.html?relay=1";
    }),
    $("#shedule2Start").click(function(){
        location.href = "shedule.html?relay=2";
    }),
    $("#shedule3Start").click(function(){
        location.href = "shedule.html?relay=3";
    }),
    $("#shedule4Start").click(function(){
        location.href = "shedule.html?relay=4";
    })
});