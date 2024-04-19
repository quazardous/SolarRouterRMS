var LaTemperature = -100;
const paramTypes = {
  bool: "bool",
  ip: "ip",
  text: "text",
  float: "float",
  int: "int",
};
const paramsList = {
  dhcp: {type: paramTypes.bool, value: false},
  adrIP: {type: paramTypes.ip, value: ""},
  gateway: {type: paramTypes.ip, value: ""},
  masque: {type: paramTypes.ip, value: ""},
  dns: {type: paramTypes.ip, value: ""},
  source: {
    type: paramTypes.text, value: "",
    get: () => document.querySelector('input[name="sources"]:checked').value,
    set: (value) => { GID(value).checked = true; }
  },
  RMSextIP: {type: paramTypes.ip, value: ""},
  EnphaseUser: {type: paramTypes.text, value: ""},
  EnphasePwd: {type: paramTypes.text, value: ""},
  EnphaseSerial: {type: paramTypes.text, value: ""},
  ShellyEmPhases: {type: paramTypes.int, value: 0},
  MQTTRepete: {type: paramTypes.int, value: 0},
  MQTTIP: {type: paramTypes.ip, value: ""},
  MQTTPort: {type: paramTypes.int, value: 0},
  MQTTUser: {type: paramTypes.text, value: ""},
  MQTTpwd: {type: paramTypes.text, value: ""},
  MQTTPrefix: {type: paramTypes.text, value: ""},
  MQTTdeviceName: {type: paramTypes.text, value: ""},
  nomRouteur: {type: paramTypes.text, value: ""},
  nomSondeFixe: {type: paramTypes.text, value: ""},
  nomSondeMobile: {type: paramTypes.text, value: ""},
  temperature: {
    type: paramTypes.float, value: 0.0,
    get: null, set: (value) => { LaTemperature = value; }
  },
  nomTemperature: {type: paramTypes.text, value: ""},
  CalibU: {type: paramTypes.int, value: 0},
  CalibI: {type: paramTypes.int, value: 0},
  TempoEDFon: {type: paramTypes.bool, value: false},
};

/**
 * @returns {Array<*>}
 */
function getParamsFromDOM() {
  let Para = [];
  for (const [id, param] of Object.entries(paramsList)) {
    if (param.get !== undefined) {
      param.value = null;
      if (param.get !== null) {
        param.value = param.get(id);
        Para.push(param.value);
      }
      continue;
    }
    switch (param.type) {
      case paramTypes.bool:
        param.value = GID(id).checked;
        break;
      case paramTypes.ip:
        param.value = ip2int(GID(id).value);
        break;
      case paramTypes.int:
        param.value = parseInt(GID(id).value);
        break;
      case paramTypes.float:
        param.value = parseFloat(GID(id).value);
        break;
      case paramTypes.text:
        param.value = GID(id).value.trim();
        break;
      default:
        param.value = GID(id).value;
        console.error("Invalid parameter type");
    }
    Para.push(param.value);
  }
  return Para;
}

/**
 * @param {Array<*>} Para 
 */
function setParamsToDOM(Para) {
  if (Para.length !== Object.keys(paramsList).length) {
    console.error("Invalid number of parameters");
    return;
  }
  let i = 0;
  for (const [id, param] of Object.entries(paramsList)) {
    let val = Para[i];
    switch (param.type) {
      case paramTypes.bool:
        val = parseInt(val) > 0;
        break;
      case paramTypes.ip:
        val = int2ip(val);
        break;
      case paramTypes.int:
        val = parseInt(val);
        break;
      case paramTypes.float:
        val = parseFloat(val);
        break;
      case paramTypes.text:
        val = val;
        break;
    }
    if (param.set !== undefined) {
      if (param.set !== null) {
        param.set(val, id);
      }
    } else {
      GID(id).value = val;
    }
    i++;
  }
}

function Init(){
  LoadParametres();
  LoadParaRouteur();
}
function LoadParametres() {
  var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
            var LesParas=this.responseText;
            var Para=LesParas.split(RS);
            setParamsToDOM(Para);           
            checkDisabled();
        }         
      };
      xhttp.open('GET', 'ParaAjax', true);
      xhttp.send();
    }
function SendValues(){
  Para = getParamsFromDOM();
  var S = "";
  for (const param of Para) {
    S += RS + param;
  }
  S="?lesparas="+clean(S);
  if ((GID("dhcp").checked || checkIP("adrIP") && checkIP("gateway")) && (!GID("MQTTRepete").checked || checkIP("MQTTIP"))){
    var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          var retour=this.responseText;
          location.reload();
        }         
      };
      xhttp.open('GET', 'ParaUpdate'+S, true);
      xhttp.send();
  }
}
function checkDisabled(){ 
  GID("adrIP").disabled=GID("dhcp").checked;
  GID("gateway").disabled=GID("dhcp").checked;
  GID("masque").disabled=GID("dhcp").checked;
  GID("dns").disabled=GID("dhcp").checked;
  GID("RMSextIP").disabled=!GID("Proxy").checked&&!GID("Enphase").checked&&!GID("SmartG").checked&&!GID("ShellyEm").checked;
  GID("MQTTIP").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTPort").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTUser").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTpwd").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID("MQTTPrefix").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID("MQTTdeviceName").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID('ligneTemperature').style.display = (LaTemperature>-100) ? "table-row" : "none";
  Source = document.querySelector('input[name="sources"]:checked').value;
  if (Source != 'Proxy') Source_data=Source;
  AdaptationSource();
}
function checkIP(id){
  var S=GID(id).value;
  var Table=S.split(".");
  var valide=true;
  if (Table.length!=4) {
    valide=false;
  }else{
    for (var i=0;i<Table.length;i++){
      if (Table[i]>255 || Table[i]<0) valide=false;
    }
  }
  if (valide){
    GID(id).style.color="black";
  } else {
    GID(id).style.color="red";
  }
  return valide;
}


function Reset(){
    var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          GID('BoutonsBas').innerHTML=this.responseText;
          setTimeout(location.reload(),3000);
        }         
      };
      xhttp.open('GET', 'restart', true);
      xhttp.send();
}
function AdaptationSource(){
    GID('ligneFixe').style.display = (Source_data=='UxIx2' || (Source_data=='ShellyEm' && GID("EnphaseSerial").value <3))? "table-row" : "none";
    if (Source_data=='UxI' && Source=='UxI' ) {
      GID('Tui').style.display="table-row";
      GID('CUi').style.display="table-row";
      GID('CuI').style.display="table-row";
    } else {
      GID('Tui').style.display="none";
      GID('CUi').style.display="none";
      GID('CuI').style.display="none";
    }
    var txtExt = "ESP-RMS";
    if (Source=='Enphase') txtExt = "Enphase-Envoy";
    if (Source=='SmartG') txtExt = "SmartGateways";
    if (Source=='ShellyEm') {
      txtExt = "Shelly Em ";
    }
    GID('labExtIp').innerHTML = txtExt;
    GID('ligneExt').style.display = (Source=='Proxy' || Source=='Enphase' || Source=='SmartG' || Source=='ShellyEm') ? "table-row" : "none";
    GID('ligneEnphaseUser').style.display = (Source=='Enphase') ? "table-row" : "none";
    GID('ligneEnphasePwd').style.display = (Source=='Enphase') ? "table-row" : "none";
    GID('ligneEnphaseSerial').style.display = (Source=='Enphase') ? "table-row" : "none"; // Numéro de serie
    GID('ligneShellyEmPhases').style.display = (Source=='ShellyEm') ? "table-row" : "none"; // Voie
}
