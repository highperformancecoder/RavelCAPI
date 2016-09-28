function sqr(x) {return x*x;};
var palette=["black","red","green","blue","magenta","cyan","yellow"];
    

var JSRavel = Module.Ravel.extend("Ravel", {
    // container of references to SVG objects
    __construct: function(svgFrame) {
        this.svgFrame=svgFrame;
        this.handle=new Array;
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

        // bind mouse actions
        var ravelframe=document.getElementById(svgFrame);
        var elem=ravelframe.parentElement;
        var offx=elem.parentElement.offsetLeft+100;
        var offy=elem.parentElement.offsetTop+100;
        this.x=0; this.y=0;
        var ravel=this;
        elem.onmousedown=function(event) {
            ravel.onMouseDown(event.clientX-offx, event.clientY-offy);
        };
        elem.onmouseup=function(event) {
            ravel.onMouseUp(event.clientX-offx, event.clientY-offy);
            ravel.draw();
        };
        elem.onmousemove=function(event) {
            if (ravel.onMouseMotion(event.clientX-offx, event.clientY-offy))
                ravel.draw();
        };
    },

    draw: function () {
        for (var i=0; i<2; i++)
        {
            if (typeof this.handle[i]=="undefined")
            {
                var h=this.handle[i]=new Object();
                h.handlePath=document.createElementNS("http://www.w3.org/2000/svg",'path');
                h.handlePath.setAttribute("fill",palette[i%palette.length]);
                h.handle=document.createElementNS("http://www.w3.org/2000/svg",'g');
                h.handle.appendChild(h.handlePath);
                document.getElementById(this.svgFrame).appendChild(h.handle);
            }
            this.drawLine(this.handle[i],this.handleX(i),this.handleY(i));
        }
    },
    
    drawLine: function(handle,x,y) {
        var f=20.0;
        var r=Math.sqrt(x*x+y*y);
        var angle=0.5*Math.asin(1/f);
        var x1=r*Math.tan(angle);
        var handlePathInfo="M0,0 A"+f*r+" "+f*r+" 0 0 0 "+x1+" "+r;
        handlePathInfo+=" h"+-2*x1;
        handlePathInfo+=" A"+f*r+" "+f*r+" 0 0 0 0,0";
        handlePathInfo+="Z";
        handle.handlePath.setAttribute("d",handlePathInfo);
        handle.handle.setAttribute("transform","rotate("+180*Math.atan2(x,y)/Math.PI+")");
    }
});

var ravel=new JSRavel("ravel");
var ravel1=new JSRavel("ravel1");
ravel1.moveHandleTo(0,75,75);

ravel.draw();
ravel1.draw();

function onunload() {
    if (ravel) ravel.delete();
    if (ravel1) ravel1.delete();
}
