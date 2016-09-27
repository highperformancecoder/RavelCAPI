function sqr(x) {return x*x;};

function drawLine(x,y) {
    var f=20.0;
    var r=Math.sqrt(x*x+y*y);
    var angle=0.5*Math.asin(1/f);
    var x1=r*Math.tan(angle);
    var handlePathInfo="M0,0 A"+f*r+" "+f*r+" 0 0 0 "+x1+" "+r;
    handlePathInfo+=" h"+-2*x1;
    handlePathInfo+=" A"+f*r+" "+f*r+" 0 0 0 0,0";
    handlePathInfo+="Z";
    var handlePath=document.createElementNS("http://www.w3.org/2000/svg",'path');
    handlePath.setAttribute("d",handlePathInfo);
    handlePath.setAttribute("fill","red");
    var handle=document.createElementNS("http://www.w3.org/2000/svg",'g');
    handle.appendChild(handlePath);
    handle.setAttribute("transform","rotate("+180*Math.atan2(y,x)/Math.PI+")");
    document.getElementById("ravel").appendChild(handle);
};

