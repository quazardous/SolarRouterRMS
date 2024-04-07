var Source="";
var Source_data="";
var RMSextIP="";
var GS=String.fromCharCode(29); //Group Separator
var RS=String.fromCharCode(30); //Record Separator
var nomSondeFixe="Sonde Fixe";
var nomSondeMobile="Sonde Mobile";
var nomTemperature="Temperature"; 
function LoadParaRouteur() {
  var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
            var LesParas=this.responseText;
            var Para=LesParas.split(RS);
            Source=Para[0];
            Source_data=Para[1];
            RMSextIP= Para[6]; 
            AdaptationSource();  
            GH("nom_R",Para[2]);
            GH("version",Para[3]);
            GH("nomSondeFixe",Para[4]);
            GH("nomSondeMobile",Para[5]); 
            nomSondeFixe=Para[4];
            nomSondeMobile=Para[5];
            nomTemperature=Para[7]; 
              
        }         
      };
      xhttp.open('GET', 'ParaRouteurAjax', true);
      xhttp.send();
}
function GID(id) { return document.getElementById(id); };
function GH(id, T) {
  if ( GID(id)){
    GID(id).innerHTML = T; }
  }
function GV(id, T) { GID(id).value = T; }
function clean(S){ //Remplace & et ? pour les envois au serveur
  let res=S.replace(/\%/g,"%25");
  res = res.replace(/\&/g, "%26");
  res = res.replace(/\#/g, "%23");
  res = res.replace(/\+/g, "%2B");
  res=res.replace(/amp;/g,"");
  return res.replace(/\?/g,"%3F");
}
function int2ip (V) {
  var ipInt=parseInt(V);
  return ( (ipInt>>>24) +'.' + (ipInt>>16 & 255) +'.' + (ipInt>>8 & 255) +'.' + (ipInt & 255) );
}
function ip2int(ip) {
  ip=ip.trim();
  return ip.split('.').reduce(function(ipInt, octet) { return (ipInt<<8) + parseInt(octet, 10)}, 0) >>> 0;
}