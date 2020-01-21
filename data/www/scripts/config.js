// Display order of devices.  Tiles will be displayed in the order below,
// any devices you don't want to see you can comment the ID. (with // e.g. `//"Solar_Heater",` )
// If the device isn't listed below is will NOT be shown.
// For a complete list returned from your particular aqualinkd instance
// use the below URL and look at the ID value for each device.
// http://aqualink.ip.address/?command=devices

var devices = [
    "Filter_Pump",
    "Spa_Mode",
    "Aux_1",
    "Aux_2",
    "Aux_3",
    "Aux_4",
    "Aux_5",
    "Aux_6",
    "Aux_7",
    "Pool_Heater",
    "Spa_Heater",
    "SWG",
    //"SWG/Percent",
    "SWG/PPM",
    "SWG/Boost",
    "Temperature/Air",
    "Temperature/Pool",
    "Temperature/Spa",
    "Pool_Water",
    "Spa_Water",
    "Freeze_Protect",
    //"Solar_Heater",
];

// Add the light program names below, the name index MUST correlate to
// the number of pulses.  ie (Blue Sea is #2 or 2 pulses)
var light_program = [
    "Voodoo Lounge - Show",
    "Blue Sea",
    "Royal Blue",
    "Afternoon Skies",
    "Aqua Green",
    "Emerald",
    "Cloud White",
    "Warm Red",
    "Flamingo",
    "Vivid Violet",
    "Sangria",
    "Twilight - Show",
    "Tranquility - Show",
    "Gemstone - Show",
    "USA - Show",
    "Mardi Gras - Show",
    "Cool Cabaret - Show"
];
/*
 *  BELOW IS NOT RELIVENT FOR simple.html or simple inteface
 *
 */

// Background image, delete or leave blank for solid color
var background_url = 'images/background.jpg';

// Reload background image every X seconds.(useful if camera snapshot)
// 0 means only load once when page loads.
//var background_reload = 10;

var body_background = "#EBEBEA";
var body_text = "#000000";

var options_pane_background = "#F5F5F5";
var options_pane_bordercolor = "#7C7C7C";
var options_slider_highlight = "#2196F3";
var options_slider_lowlight = "#D3D3D3";

var head_background = "#2B6A8F";
var head_text = "#FFFFFF)";
var error_background = "#8F2B2B";

var tile_background = "#DCDCDC";
var tile_text = "#6E6E6E";
var tile_on_background = "#FFFFFF";
var tile_on_text = "#000000";
var tile_status_text = "#575757";

// Dark colors
// var body_background = "#000000";
// var tile_background = "#646464";
// var tile_text = "#B9B9B9";
// var tile_status_text = "#B2B2B2";
// var head_background = "#000D53";