function sqr(x) {return x*x;};

var JSRavel = Module.Ravel.extend("Ravel", {
    __construct: function(svgFrame) {
        this.svgFrame=svgFrame;
        this.__parent.__construct.call(this);

        this.rescale(100);

        var yearData=new Module.VectorString();
        yearData.push_back("1980");
        yearData.push_back("1990");
        this.addHandle("Year",yearData);
        var genderData=new Module.VectorString();
        genderData.push_back("male");
        genderData.push_back("female");
        this.addHandle("Gender",genderData);
    },

    draw: function () {
        this.drawLine(this.handleX(0),this.handleY(0));
        this.drawLine(this.handleX(1),this.handleY(1));
    },
    
    drawLine: function(x,y) {
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
    }
});


var ravel=new JSRavel("ravel");

var ravel1=new JSRavel("ravel1");
ravel1.moveHandleTo(0,75,75);

ravel.draw();
ravel1.draw();
