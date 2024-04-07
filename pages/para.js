var LaTemperature = -100;
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
            GID("dhcp").checked = Para[0]==1  ? true:false;
            GID("adrIP").value=int2ip(Para[1]);
            GID("gateway").value=int2ip(Para[2]);
            GID("masque").value=int2ip(Para[3]);
            GID("dns").value=int2ip(Para[4]);
            GID(Para[5]).checked = true;
            GID("RMSextIP").value=int2ip(Para[6]);
            GID("EnphaseUser").value=Para[7];
            GID("EnphasePwd").value=Para[8];
            GID("EnphaseSerial").value=Para[9];
            GID("MQTTRepete").value = Para[10];
            GID("MQTTIP").value=int2ip(Para[11]);
            GID("MQTTPort").value=Para[12];
            GID("MQTTUser").value=Para[13];
            GID("MQTTpwd").value=Para[14];
            GID("MQTTPrefix").value=Para[15];
            GID("MQTTdeviceName").value=Para[16];
            GID("nomRouteur").value=Para[17];
            GID("nomSondeFixe").value=Para[18];
            GID("nomSondeMobile").value=Para[19];
            LaTemperature=parseInt(Para[20]);
            GID("nomTemperature").value=Para[21];
            GID("CalibU").value=Para[22];
            GID("CalibI").value=Para[23];
            GID("TempoEDFon").checked = Para[24]==1  ? true:false;
            
            checkDisabled();
        }         
      };
      xhttp.open('GET', 'ParaAjax', true);
      xhttp.send();
    }
function SendValues(){
  var dhcp=GID("dhcp").checked ? 1:0;
  var TempoEDFon=GID("TempoEDFon").checked ? 1:0;
  var Source_new=document.querySelector('input[name="sources"]:checked').value;
  var S=dhcp+RS+ ip2int(GID("adrIP").value)+RS+ ip2int(GID("gateway").value);
  S +=RS+ip2int(GID("masque").value)+RS+ ip2int(GID("dns").value)
  S +=RS+Source_new+RS+ ip2int(GID("RMSextIP").value)+ RS+GID("EnphaseUser").value.trim()+RS+GID("EnphasePwd").value.trim()+RS+GID("EnphaseSerial").value.trim();
  S +=RS+GID("MQTTRepete").value +RS+ip2int(GID("MQTTIP").value) +RS+GID("MQTTPort").value +RS+GID("MQTTUser").value.trim()+RS+GID("MQTTpwd").value.trim();
  S +=RS+GID("MQTTPrefix").value.trim()+RS+GID("MQTTdeviceName").value.trim()+RS+GID("nomRouteur").value.trim()+RS+GID("nomSondeFixe").value.trim()+RS+GID("nomSondeMobile").value.trim();
  S +=RS+GID("nomTemperature").value.trim();
  S +=RS+GID("CalibU").value+RS+GID("CalibI").value + RS + TempoEDFon;
  S="?lesparas="+clean(S);
  if ((GID("dhcp").checked ||  checkIP("adrIP")&&checkIP("gateway"))   && (!GID("MQTTRepete").checked ||  checkIP("MQTTIP"))){
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
  GID("RMSextIP").disabled=!GID("Ext").checked&&!GID("Enphase").checked&&!GID("SmartG").checked&&!GID("ShellyEm").checked;
  GID("MQTTIP").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTPort").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTUser").disabled=GID("MQTTRepete").value ==0?true:false;
  GID("MQTTpwd").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID("MQTTPrefix").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID("MQTTdeviceName").disabled=GID("MQTTRepete").value ==0?true:false; 
  GID('ligneTemperature').style.display = (LaTemperature>-100) ? "table-row" : "none";
  Source = document.querySelector('input[name="sources"]:checked').value;
  if (Source !='Ext') Source_data=Source;
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
    var lab_enphaseShelly= "Numéro série passerelle IQ Enphase : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span>";
    if (Source=='ShellyEm') {
      txtExt = "Shelly Em ";
      lab_enphaseShelly="Monophasé : Numéro de voie (0 ou 1) mesurant l'entrée du courant maison<br>Triphasé : mettre 3";
    }
    GID('labExtIp').innerHTML = txtExt;
    GID('label_enphase_shelly').innerHTML = lab_enphaseShelly;
    GID('ligneExt').style.display = (Source=='Ext' || Source=='Enphase' || Source=='SmartG' || Source=='ShellyEm') ? "table-row" : "none";
    GID('ligneEnphaseUser').style.display = (Source=='Enphase') ? "table-row" : "none";
    GID('ligneEnphasePwd').style.display = (Source=='Enphase') ? "table-row" : "none";
    GID('ligneEnphaseSerial').style.display = (Source=='Enphase' || Source=='ShellyEm') ? "table-row" : "none"; //Numéro de serie ou voie
}
