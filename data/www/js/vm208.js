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
    update_auth_settings(), update_network_settings(), update_email_settings(), update_names(), update_notif_settings(), UpdateSheduleFields(), updateWifiNetworkFieldState(), updateEthNetworkFieldsState()
}

function update_auth_settings() {
    var e = $("#login_field"),
        t = $("#password_field");
    e.val(json.USERNAME);
}

function update_email_settings() {
    var e = $("#smtp_server"),
        t = $("#smtp_port"),
        a = $("#smtp_user"),
        n = $("#smtp_pass"),
        s = $("#smtp_tls");
    //e.val(json.email.smtpserver), t.val(json.email.smtpport), a.val(json.email.username), n.val(json.email.password), s.prop("checked", json.email.usessltls)
}

function update_network_settings() {
    var a = $("#dhcp_enabled_eth_checkbox"),
        n = $("#value_eth_ipaddress"),
        s = $("#value_eth_gateway"),
        l = $("#value_eth_subnetmask"),
        o = $("#value_eth_primarydns"),
        i = $("#value_eth_secondarydns"),
        b = $("#dhcp_enabled_wifi_checkbox"),
        c = $("#value_wifi_ipaddress"),
        d = $("#value_wifi_gateway"),
        e = $("#value_wifi_subnetmask"),
        f = $("#value_wifi_primarydns"),
        g = $("#value_wifi_secondarydns");
        a.prop("checked", json.ETH_DHCPEN), n.val(json.ETH_IPADDR), s.val(json.ETH_GATEWAY), l.val(json.ETH_SUBNET), o.val(json.ETH_PRIMARYDNS), i.val(json.ETH_SECONDARYDNS),
        b.prop("checked", json.WIFI_DHCPEN), c.val(json.WIFI_IPADDR), d.val(json.WIFI_GATEWAY), e.val(json.WIFI_SUBNET), f.val(json.WIFI_PRIMARYDNS), g.val(json.WIFI_SECONDARYDNS);
}

function update_names() {
    var e = $("#name_card"),
        t = $("#name_relay1"),
        a = $("#name_relay2"),
        n = $("#name_relay3"),
        s = $("#name_relay4"),
        l = $("#pulse_relay1"),
        o = $("#pulse_relay2"),
        i = $("#pulse_relay3"),
        r = $("#pulse_relay4"),
        u = $("#name_input1"),
        c = $("#name_input2"),
        d = $("#name_input3"),
        m = $("#name_input4"),
        p = $("#name_analog"),
        v = $("#name_custom_js"),
        _ = $("#name_custom_css");
    //t.val(json.io.relays[0].name), a.val(json.io.relays[1].name), n.val(json.io.relays[2].name), s.val(json.io.relays[3].name), $(".r1Name").html(json.io.relays[0].name), $(".r2Name").html(json.io.relays[1].name), $(".r3Name").html(json.io.relays[2].name), $(".r4Name").html(json.io.relays[3].name), u.val(json.io.inputs[0].name), c.val(json.io.inputs[1].name), d.val(json.io.inputs[2].name), m.val(json.io.inputs[3].name), l.val(json.io.relays[0].time), o.val(json.io.relays[1].time), i.val(json.io.relays[2].time), r.val(json.io.relays[3].time), $("#i1Name").html(json.io.inputs[0].name), $("#i2Name").html(json.io.inputs[1].name), $("#i3Name").html(json.io.inputs[2].name), $("#i4Name").html(json.io.inputs[3].name), p.val(json.io.analog.name), v.val(json.customjs), _.val(json.customcss), e.val(json.cardname)
}

function update_notif_settings() {
    var e, t = notif_select.val();
    //t || (t = 0), e = json.notifications[t], $("#notif_enabled_checkbox").prop("checked", e.enable), $("#recipient_field").val(e.recipients), $("#alarmvalue_field").val(json.io.analog.alarmvalue), 9 == notif_select.val() ? $("#alarmvalue_div").show() : $("#alarmvalue_div").hide()
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

function sendEmailSettings() {
    var e = {
        smtpserver: $("#smtp_server").val(),
        smtpport: $("#smtp_port").val(),
        user: $("#smtp_user").val(),
        password: $("#smtp_pass").val(),
        tls: $("#smtp_tls").prop("checked") ? 1 : 0
    };
    $("#sendMailSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/email_settings",
        dataType: "text",
        data: e,
        success: function(e) {
            "OK" == e ? ($("#sendMailSettingsButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
        }
    })
}

function sendRegenerateAPIKey() {
    $.ajax({
        type: "GET",
        url: "/regenerate_api",
        dataType: "text",
        success: function(e) {
            var t = $.parseJSON(e);
            $("#api_key").val(t.ApiKey)
        }
    })
}

function sendRequestBootloader() {
    $.ajax({
        type: "GET",
        url: "/bootloader",
        dataType: "text",
        success: function() {}
    })
}

function sendAlarm() {
	
	if(validateEmail($("#recipient_field").val()))
	{
		var e = {
			alarm: $("#notification_select").val(),
			enable: $("#notif_enabled_checkbox").is(":checked") ? 1 : 0,
			recipient: $("#recipient_field").val(),
			alarmvalue: $("#alarmvalue_field").val()
		};
		$("#sendAlarmButton").html("SAVING...");
		$.ajax({
			type: "POST",
			url: "/alarm_settings",
			dataType: "text",
			data: e,
			success: function(e) {
				try {
					"OK" == e ? ($("#sendAlarmButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
				} catch (t) {
					console.log(t)
				}
			}
		})
	}
	else
	{
		$("#sendAlarmButton").html("INVALID EMAIL ADDRESS");
		setTimeout(function(){$("#sendAlarmButton").html("Save");}, 5000);
	}
}

function validateEmail(email) {
	var emails = email.split(";");
	var valid = false;
	var re = /^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/i;
	for(var i = emails.length; i>0;i--)
	{
		if(!re.test(emails[i-1]))
		{
			valid = false;
			break;
		}
		else
		{
			valid = true;
		}
		
	}
    return valid
}

function sendAuthSettings() {
    var e = {
        login: $("#login_field").val(),
        password: $("#password_field").val()
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

function ValidateIPaddress(e) {
    return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(e) ? !0 : (alert("You have entered an invalid IP address!"), !1)
}

function sendEthNetworkSettings() {
    if (ValidateIPaddress($("#value_eth_ipaddress").val()) && ValidateIPaddress($("#value_eth_gateway").val()) && ValidateIPaddress($("#value_eth_subnetmask").val()) && ValidateIPaddress($("#value_eth_primarydns").val()) && ValidateIPaddress($("#value_eth_secondarydns").val())) {
			var e = {
				dhcpenable: $("#dhcp_enabled_eth_checkbox").is(":checked") ? true : false,
				ipaddress: $("#value_eth_ipaddress").val(),
				gateway: $("#value_eth_gateway").val(),
				subnetmask: $("#value_eth_subnetmask").val(),
				primarydns: $("#value_eth_primarydns").val(),
				secondarydns: $("#value_eth_secondarydns").val()
			};
			
			$("#sendNetworkSettingsButton").html("SAVING..."); {
				$.ajax({
					type: "POST",
					url: "/eth_ip_save",
					dataType: "text",
					data: e,
					success: function(e) {
						try {
                            $("#sendNetworkSettingsButton").html("SAVED");
                            requestSettings(); 
						} catch (t) {
							console.log(t)
						}
					}
				}).fail(function(data){
                    console.log("FAILED TO SAVE NETWORKSETTINGS");
                })
			}
    } else $("#sendNetworkSettingsButton").html("INVALID IP ADDRESSES")
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

function sendNames() {
    /*var e = {
        r1: $("#name_relay1").val(),
        r2: $("#name_relay2").val(),
        r3: $("#name_relay3").val(),
        r4: $("#name_relay4").val(),
        p1: $("#pulse_relay1").val(),
        p2: $("#pulse_relay2").val(),
        p3: $("#pulse_relay3").val(),
        p4: $("#pulse_relay4").val(),
        i1: $("#name_input1").val(),
        i2: $("#name_input2").val(),
        i3: $("#name_input3").val(),
        i4: $("#name_input4").val(),
        an: $("#name_analog").val(),
        js: $("#name_custom_js").val(),
        css: $("#name_custom_css").val(),
        cardname: $("#name_card").val()
    };
	
    $("#sendNameSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/alarm",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                "OK" == e ? ($("#sendNameSettingsButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
            } catch (t) {
                console.log(t)
            }
        }
    })*/
    var e = {
        state: $("#shedule_state").val(),
        relais: $("#shedule_relais").val(),
        day: $("#shedule_day").val(),
        hour: $("#shedule_hour").val(),
        minute: $("#shedule_minute").val(),
        enable: $("#shedule_enable").val()
    };
	
    $("#sendNameSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/alarm",
        dataType: "text",
        data: e,
        success: function(e) {
        }
    })
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
			}
		})
	}
}

function updateSettings(e) {
    e.authentication.login
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

function testEmailSettings() {
    $.ajax({
        type: "POST",
        url: "/testmail",
        dataType: "text",
        success: function(e) {
            try {
                var jsonMail = $.parseJSON(e);
                $("#testMailSettingsButton").html(jsonMail.status);
            } catch (t) {
                console.log(t)
            }
        }
    })
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
}

function enableButtons() {
    var e;
    for (e = 1; 12 >= e; e++) $("#relay" + e + "Status").removeClass("pure-button-disabled"), $("#mosfet" + e + "Status").removeClass("pure-button-disabled"), $("#pulse" + e + "Start").removeClass("pure-button-disabled"), $("#timer" + e + "Start").removeClass("pure-button-disabled")
}
var json, notif_select = new Object;
$(document).ready(function() {
    requestBoardInfo(),requestSettings(), notif_select = $("#notification_select"), notif_select.change(update_notif_settings), $("#splashscreen").delay(750).fadeOut(500), enableButtons(),updateEthNetworkFieldsState(),updateWifiNetworkFieldState()
});
var current_slide = 0;
$(function() {
    $("#sendMailButton").removeAttr("disabled"), timerRelayEvent(), $("#relay1Status").click(function() {
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
    }), $("#sendAuthSettingsButton").click(function() {
        return sendAuthSettings(), !1
    }), $("#sendWifiNetworkSettingsButton").click(function() {
        return sendWifiNetworkSettings(), !1
    }), $("#sendEthNetworkSettingsButton").click(function() {
        return sendEthNetworkSettings(), !1
    }), $("#sendMailSettingsButton").click(function() {
        return sendEmailSettings(), !1
    }), $("#sendNameSettingsButton").click(function() {
        return sendNames(), !1
    }), $("#sendAlarmButton").click(function() {
        return sendAlarm(), !1
    }), $("#regeneratekeyButton").click(function() {
        return sendRegenerateAPIKey(), !1
    }), $("#requestBootloader").click(function() {
        return sendRequestBootloader(), !1
    }), $("#testMailSettingsButton").click(function() {
		$("#testMailSettingsButton").html("Sending...")
        $.ajax({
        type: "POST",
        url: "/testmail",
        dataType: "text",
        success: function(e) {
            try {
                var jsonMail = $.parseJSON(e);
                var jsonMail = $.parseJSON(e);
                $("#testMailSettingsButton").html(jsonMail.status)
                setTimeout(function(){$("#testMailSettingsButton").html("Test Mail Settings");}, 5000);
            } catch (t) {
                console.log(t)
            }
        }
    })
    }), $("#shedule_state").change(function(){UpdateSheduleFields()}),
    $("#shedule_relais").change(function(){UpdateSheduleFields()}),
    $("#shedule_day").change(function(){UpdateSheduleFields()}),
    $("#dhcp_enabled_wifi_checkbox").change(function(){
       updateWifiNetworkFieldState();
    }),
    $("#dhcp_enabled_eth_checkbox").change(function(){
        updateEthNetworkFieldsState();
    })
});

function updateEthNetworkFieldsState()
{
    if($("#dhcp_enabled_eth_checkbox").is(":checked"))
        {
            $("#value_eth_ipaddress").attr("disabled",true);
            $("#value_eth_gateway").attr("disabled",true);
            $("#value_eth_subnetmask").attr("disabled",true);
            $("#value_eth_primarydns").attr("disabled",true);
            $("#value_eth_secondarydns").attr("disabled",true);
        }
        else
        {
            $("#value_eth_ipaddress").attr("disabled",false);
            $("#value_eth_gateway").attr("disabled",false);
            $("#value_eth_subnetmask").attr("disabled",false);
            $("#value_eth_primarydns").attr("disabled",false);
            $("#value_eth_secondarydns").attr("disabled",false);
            
        }
}

function updateWifiNetworkFieldState()
{
     if($("#dhcp_enabled_wifi_checkbox").is(":checked"))
        {
            $("#value_wifi_ipaddress").attr("disabled",true);
            $("#value_wifi_gateway").attr("disabled",true);
            $("#value_wifi_subnetmask").attr("disabled",true);
            $("#value_wifi_primarydns").attr("disabled",true);
            $("#value_wifi_secondarydns").attr("disabled",true);
        }
        else
        {
            $("#value_wifi_ipaddress").attr("disabled",false);
            $("#value_wifi_gateway").attr("disabled",false);
            $("#value_wifi_subnetmask").attr("disabled",false);
            $("#value_wifi_primarydns").attr("disabled",false);
            $("#value_wifi_secondarydns").attr("disabled",false);            
        }
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
					console.log(t);
                    timerRelayEvent();
				}
			}
		})
	}
}

function UpdateSheduleFields()
{
    var state = parseInt($("#shedule_state").val(),10);
    var relais = $("#shedule_relais").val() - 1;
    var day = parseInt($("#shedule_day").val(),10);
    var alarm = (day *2) + state;
    var hour = json.Channels[relais].alarms[alarm].hour;
    var minute = json.Channels[relais].alarms[alarm].minute;
    var enabled = json.Channels[relais].alarms[alarm].enabled;    
    $("#shedule_hour").val(hour);
    $("#shedule_minute").val(minute);
    $("#shedule_enable").val(enabled?"1":"0");
    
}