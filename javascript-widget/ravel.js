function sqr(x) {return x*x;};
var palette=["black","red","green","blue","magenta","cyan","yellow"];
    

var JSRavel = Module.Ravel.extend("Ravel", {
    // container of references to SVG objects
    __construct: function(svgFrame) {
        this.svgFrame=svgFrame;
        this.handle=new Array;
        this.__parent.__construct.call(this);

        this.rescale(140);

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

    theta: 0.05, //radians
    rotator: function(r,colour) {
        var dx=r*Math.sin(this.theta), dy=r*(Math.cos(this.theta)-1);
        var rotatorArrowShaft="M"+-dx+","+dy+"A"+r+","+r+" 0 0 0 "+dx+","+dy;
        var rotatorArrowHead="M0,0L-1,-1v2Z";

        var rotatorShaft=document.createElementNS("http://www.w3.org/2000/svg",'path');
        rotatorShaft.setAttribute("d", rotatorArrowShaft);
        rotatorShaft.setAttribute("stroke", colour);
        rotatorShaft.setAttribute("stroke-width",2);
        rotatorShaft.setAttribute("fill", "none");
        rotatorShaft.setAttribute("transform","translate(0,"+r+")");
        
        var rotatorHead=document.createElementNS("http://www.w3.org/2000/svg",'path');
        rotatorHead.setAttribute("d", rotatorArrowHead);
        rotatorHead.setAttribute("transform","rotate("+-this.theta*180/Math.PI+") translate(0,"+r+") scale(5,5)");
        rotatorHead.setAttribute("fill", colour);
        var rotator=document.createElementNS("http://www.w3.org/2000/svg",'g');
        rotator.appendChild(rotatorHead);
        rotator.appendChild(rotatorShaft);
        return rotator;
    },
    
    draw: function () {
        for (var i=0; i<2; i++)
        {
            var x=this.handleX(i), y=this.handleY(i);
            if (typeof this.handle[i]=="undefined")
            {
                var h=this.handle[i]=new Object();
                var handlePath=document.createElementNS("http://www.w3.org/2000/svg",'path');
                handlePath.setAttribute("fill",palette[i%palette.length]);
                h.handle=document.createElementNS("http://www.w3.org/2000/svg",'g');
                h.handle.appendChild(handlePath);
                document.getElementById(this.svgFrame).appendChild(h.handle);
                var f=20.0;
                var r=0.9*Math.sqrt(x*x+y*y);
                var angle=0.5*Math.asin(1/f);
                var x1=r*Math.tan(angle);
                var handlePathInfo="M0,0 A"+f*r+" "+f*r+" 0 0 0 "+x1+" "+r;
                //handlePathInfo+=" h"+-2*x1;
                handlePathInfo+=" h"+x1+" L0,"+1.1*r+
                    " L"+-2*x1+","+r+" h"+x1; //arrow
                handlePathInfo+=" A"+f*r+" "+f*r+" 0 0 0 0 0";
                handlePathInfo+="Z";
                handlePath.setAttribute("d",handlePathInfo);

                var rotator1=this.rotator(1.05*r, palette[i%palette.length]);
                rotator1.setAttribute("transform","rotate("+-2*this.theta*180/Math.PI+")");
                var rotator2=this.rotator(1.05*r, palette[i%palette.length]);
                rotator2.setAttribute("transform","scale(-1,1) rotate("+-2*this.theta*180/Math.PI+")");

                var collapsor=document.createElementNS("http://www.w3.org/2000/svg",'path');
                collapsor.setAttribute("d","M0,0l0,-5h1l-1,-1l-1,1h1Z");
                collapsor.setAttribute("transform","translate(0,"+1.05*r+") scale(1.5,1.5)");
                collapsor.setAttribute("stroke","white");
                collapsor.setAttribute("fill","white");
                
                // group rotator arrows to allow opacity to be applied to the group
                h.rotator=document.createElementNS("http://www.w3.org/2000/svg",'g');
                h.handle.appendChild(h.rotator);
                h.rotator.appendChild(rotator1);
                h.rotator.appendChild(rotator2);
                h.rotator.appendChild(collapsor);
            }
            this.handle[i].handle.setAttribute
            ("transform",
             "rotate("+180*Math.atan2(x,y)/Math.PI+")");
        }
    },
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
