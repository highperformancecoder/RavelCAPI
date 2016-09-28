function sqr(x) {return x*x;};

function JSRavel(svgFrame) {
    this.svgFrame=svgFrame;
    var ravel=new Module.Ravel();
    this.ravel=ravel;
    ravel.rescale(100);
    for (var i in ravel)
        this[i]=ravel[i];

    var yearData=new Module.VectorString();
    yearData.push_back("1980");
    yearData.push_back("1990");
    ravel.addHandle("Year",yearData);
    var genderData=new Module.VectorString();
    genderData.push_back("male");
    genderData.push_back("female");
    ravel.addHandle("Gender",genderData);
};

JSRavel.prototype.draw = function () {
        this.drawLine(this.ravel.handleX(0),this.ravel.handleY(0));
        this.drawLine(this.ravel.handleX(1),this.ravel.handleY(1));
    }
    
JSRavel.prototype.drawLine = function(x,y) {
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
        document.getElementById(this.svgFrame).appendChild(handle);
    };


var ravel=new JSRavel("ravel");

var ravel1=new JSRavel("ravel1");
ravel1.ravel.moveHandleTo(0,75,75);

ravel.draw();
ravel1.draw();
