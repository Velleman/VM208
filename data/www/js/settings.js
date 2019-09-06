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
    update_auth_settings(), update_network_settings(), update_email_settings(), update_names(), update_notif_settings(), updateWifiNetworkFieldState(), updateEthNetworkFieldsState(), updateTimeSettings()
}

function update_auth_settings() {
    var e = $("#login_field"),
        t = $("#password_field");
    e.val(json.USERNAME);
}

function update_email_settings() {
        $("#smtp_server").val(json.email.smtpserver);
        $("#smtp_port").val(json.email.smtpport);
        $("#smtp_user").val(json.email.username);
        $("#smtp_recipient").val(json.test);
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
    var e = $("#name_card").val(json.BOARDNAME);
    for (var i = 1; i <= 12; i++) {
        $("#name_relay" + i).val(json.Channels[i - 1].name);
    }
    $("#name_input1").val(json.NAME_INPUT);
    $("#name_mosfet1").val(json.NAME_MOSFET1);
    $("#name_mosfet2").val(json.NAME_MOSFET2);
}

function update_notif_settings() {
    $("#AlarmBoot").prop("checked", json.NOTIFICATION_BOOT);
    $("#AlarmExt").prop("checked", json.NOTIFICATION_EXT_DIS);
    $("#AlarmInput").prop("checked", json.NOTIFICATION_INPUT_CHANGE);
    $("#AlarmManInput").prop("checked", json.NOTIFICATION_MANUAL_INPUT);
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
        recipeint: $("#smtp_recipient").val(),
        title: $("#smtp_title").val()
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
    var e = {
        boot: $("#AlarmBoot").is(":checked") ? true : false,
        ext: $("#AlarmExt").is(":checked") ? true : false,
        input: $("#AlarmInput").is(":checked") ? true : false,
        button: $("#AlarmManInput").is(":checked") ? true : false,
    };
    $("#sendAlarmButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/notif_setting",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                "OK" == e ? ($("#sendAlarmButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
            } catch (t) {
                console.log(t)
            }
        }
    });
}

function validateEmail(email) {
    var emails = email.split(";");
    var valid = false;
    var re = /^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/i;
    for (var i = emails.length; i > 0; i--) {
        if (!re.test(emails[i - 1])) {
            valid = false;
            break;
        } else {
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
            }).fail(function(data) {
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
    var e = {
        r1: $("#name_relay1").val(),
        r2: $("#name_relay2").val(),
        r3: $("#name_relay3").val(),
        r4: $("#name_relay4").val(),
        r5: $("#name_relay5").val(),
        r6: $("#name_relay6").val(),
        r7: $("#name_relay7").val(),
        r8: $("#name_relay8").val(),
        r9: $("#name_relay9").val(),
        r10: $("#name_relay10").val(),
        r11: $("#name_relay11").val(),
        r12: $("#name_relay12").val(),
        m1: $("#name_mosfet1").val(),
        m2: $("#name_mosfet2").val(),
        i1: $("#name_input1").val(),
        cardname: $("#name_card").val()
    };

    $("#sendNameSettingsButton").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/names",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                "OK" == e ? ($("#sendNameSettingsButton").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
            } catch (t) {
                console.log(t)
            }
        }
    });

}

function sliderOnInitialized() {
    $("body").css("display", "block")
}

function timerRelayEvent() {

    if (location.pathname == "/index.html" || location.pathname == "/") {
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
        t.isExtConnected ? $("#extRelay").show() : $("#extRelay").hide();
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

function requestBoardInfo() {
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

function enableButtons() {
    var e;
    for (e = 1; 12 >= e; e++) $("#relay" + e + "Status").removeClass("pure-button-disabled"), $("#mosfet" + e + "Status").removeClass("pure-button-disabled"), $("#pulse" + e + "Start").removeClass("pure-button-disabled"), $("#timer" + e + "Start").removeClass("pure-button-disabled")
}
var json, notif_select = new Object;
$(document).ready(function() {
    requestBoardInfo(), requestSettings(), $("#splashscreen").delay(750).fadeOut(500), enableButtons(), updateEthNetworkFieldsState(), updateWifiNetworkFieldState()
});
var current_slide = 0;
$(function() {
    $("#sendMailButton").removeAttr("disabled"),
        $("#sendAuthSettingsButton").click(function() {
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
                        setTimeout(function() {
                            $("#testMailSettingsButton").html("Test Mail Settings");
                        }, 5000);
                    } catch (t) {
                        console.log(t)
                    }
                }
            })
        }),
        $("#dhcp_enabled_wifi_checkbox").change(function() {
            updateWifiNetworkFieldState();
        }),
        $("#dhcp_enabled_eth_checkbox").change(function() {
            updateEthNetworkFieldsState();
        }),
        $("#sendTimeSettings").click(function() {
            sendTimeSettings();
        }),
        $("form").submit(function(evt) {
            evt.preventDefault();
            if (confirm("This will turn off all the relays\r\nDo you want to proceed?")) {
                var formData = new FormData($(this)[0]);
                $.ajax({
                    url: '/doUpdate',
                    type: 'POST',
                    data: formData,
                    async: false,
                    cache: false,
                    contentType: false,
                    enctype: 'multipart/form-data',
                    processData: false,
                    success: function(response) {
                        //alert(response);
                    }
                });
            }
            return false;
        });
});

function sendTimeSettings() {
    var e = {
        timezone: $("#timezone_dropdown").val() * 3600,
        dst: $("#DST_enable").prop("checked")
    };

    $("#sendTimeSettings").html("SAVING...");
    $.ajax({
        type: "POST",
        url: "/time_settings",
        dataType: "text",
        data: e,
        success: function(e) {
            try {
                "OK" == e ? ($("#sendTimeSettings").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
            } catch (t) {
                console.log(t)
            }
        }
    })
}

function updateTimeSettings() {
    var timezone = json.TIMEZONE;
    var DST = json.DST;
    timezone /= 3600;
    $("#timezone_dropdown").val(timezone);
    $("#DST_enable").prop("checked", (DST == 3600));
}

function updateEthNetworkFieldsState() {
    if ($("#dhcp_enabled_eth_checkbox").is(":checked")) {
        $("#value_eth_ipaddress").attr("disabled", true);
        $("#value_eth_gateway").attr("disabled", true);
        $("#value_eth_subnetmask").attr("disabled", true);
        $("#value_eth_primarydns").attr("disabled", true);
        $("#value_eth_secondarydns").attr("disabled", true);
    } else {
        $("#value_eth_ipaddress").attr("disabled", false);
        $("#value_eth_gateway").attr("disabled", false);
        $("#value_eth_subnetmask").attr("disabled", false);
        $("#value_eth_primarydns").attr("disabled", false);
        $("#value_eth_secondarydns").attr("disabled", false);

    }
}

function updateWifiNetworkFieldState() {
    if ($("#dhcp_enabled_wifi_checkbox").is(":checked")) {
        $("#value_wifi_ipaddress").attr("disabled", true);
        $("#value_wifi_gateway").attr("disabled", true);
        $("#value_wifi_subnetmask").attr("disabled", true);
        $("#value_wifi_primarydns").attr("disabled", true);
        $("#value_wifi_secondarydns").attr("disabled", true);
    } else {
        $("#value_wifi_ipaddress").attr("disabled", false);
        $("#value_wifi_gateway").attr("disabled", false);
        $("#value_wifi_subnetmask").attr("disabled", false);
        $("#value_wifi_primarydns").attr("disabled", false);
        $("#value_wifi_secondarydns").attr("disabled", false);
    }
}

function confirmUpdate() {
    if (confirm("This will turn off all the relays\r\nDo you want to proceed?")) {
        //method="POST" action='/doUpdate'
        var formData = new FormData($(this)[0]);
        $.ajax({
            xhr: function() {
                var xhr = new window.XMLHttpRequest();
                //Upload progress
                xhr.upload.addEventListener("progress", function(evt) {
                    if (evt.lengthComputable) {
                        var percentComplete = evt.loaded / evt.total;
                        //Do something with upload progress
                        console.log(percentComplete);
                    }
                }, false);
                return xhr;
            },
            type: "POST",
            url: "/doUpdate",
            contentType: "multipart/form-data:",
            dataType: "text",
            data: e,
            success: function(e) {
                try {
                    "OK" == e ? ($("#sendTimeSettings").html("SAVED"), requestSettings()) : console.log("FAILED TO SAVE EMAILSETTINGS")
                } catch (t) {
                    console.log(t)
                }
            }
        });
    }
}