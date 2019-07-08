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