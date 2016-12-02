var table=""; // selected table

function sqr(x) {return x*x;};
var palette=["black","red","green","blue","magenta","cyan","yellow"];

/*
  HTMLCanvas is an array of HTML canvas contexts. Add a new context using the method newCanvas */
var HTMLcanvas = [];
HTMLcanvas.newCanvas = function(id) {
    var canvas = document.getElementById(id);
    if (canvas.getContext) {
        HTMLcanvas.push(canvas.getContext('2d'));
    }
}



var JSRavel = Module.Ravel.extend("Ravel", {
    // container of references to SVG objects
    __construct: function(svgFrame) {
        this.svgFrame=svgFrame;
        this.handle=new Array;
        this.__parent.__construct.call(this);

        this.rescale(140);

//        var yearData=new Module.VectorString();
//        yearData.push_back("1980");
//        yearData.push_back("1990");
//        this.addHandle("Year",yearData);
//        var genderData=new Module.VectorString();
//        genderData.push_back("male");
//        genderData.push_back("female");
//        this.addHandle("Gender",genderData);

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

    toggleRotator: function(handleIdx,opacity) {
        this.handle[handleIdx].rotator.setAttribute("opacity",opacity);
    },

    clearHandles: function() {
        var svg=document.getElementById(this.svgFrame);
        while (svg.hasChildNodes())
            svg.removeChild(svg.firstChild);
        this.clear();
        this.handle=new Array;
    },
    
    draw: function () {
        for (var i=0; i<this.numHandles(); i++)
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

                // rotator arrows
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
                var rotator=document.createElementNS("http://www.w3.org/2000/svg",'g');
                h.handle.appendChild(rotator);
                rotator.appendChild(rotator1);
                rotator.appendChild(rotator2);
                rotator.appendChild(collapsor);
                rotator.setAttribute("opacity",0);
                // handle toggling rotator display when entering/leaving the handle
                // nb note a function returning function call to avoid
                // hoisting problems, as per Crocker's suggestion on
                // page 39 of "Javascript: the good parts"
                h.handle.onmouseenter=function(r) {return function() {r.setAttribute("opacity",1);}}(rotator);
                h.handle.onmouseleave=function(r) {return function() {r.setAttribute("opacity",0);}}(rotator);

                // handle description
                h.description=document.createElementNS("http://www.w3.org/2000/svg",'text');
                h.description.innerHTML=this.handles(i).description;
                h.description.setAttribute("fill",palette[i%palette.length]);
                h.description.setAttribute("transform","scale(-1,1)");
                document.getElementById(this.svgFrame).appendChild(h.description);
             //   h.handle.appendChild(description);
            }
            this.handle[i].handle.setAttribute
            ("transform",
             "rotate("+180*Math.atan2(x,y)/Math.PI+")");
            this.handle[i].description.setAttribute("x",this.handles(i).labelAnchor().x);
            this.handle[i].description.setAttribute("y",this.handles(i).labelAnchor().y);
            switch (this.handles(i).labelAnchor().anchor)
            {
                case Module.Anchor.nw: case Module.Anchor.sw:
                this.handle[i].description.setAttribute("text-anchor","start");
                break;
                case Module.Anchor.ne: case Module.Anchor.se:
                this.handle[i].description.setAttribute("text-anchor","end");
                break;
            }
        }
    },
});

var xhttp = new XMLHttpRequest();

// populate table selector
xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        var response = eval(this.responseText);
        response.unshift("-"); // a dummy table name to indicate unselected
        var tableSelector=document.getElementById("tableSelector");
        for (var i=0; i<response.length; ++i)
        {
            var option=document.createElement("option");
            tableSelector.appendChild(option);
            option.setAttribute("value",response[i]);
            option.innerHTML=response[i];
        }
    }
}

xhttp.open("GET","/mySqlService.php/axes");
xhttp.send();

var ravel=new JSRavel("ravel");

//ravel.draw();

function setTable(name) {
    table=name;
    ravel.clearHandles();
    
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            var axes = eval(this.responseText);
            for (var i=0; i<axes.length; ++i)
            {
                var sliceLabelReq=new XMLHttpRequest();
                sliceLabelReq.axis=axes[i];
                sliceLabelReq.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        var sliceLabels=new Module.VectorString();
                        var sliceLabelData=eval(this.responseText);
                        for (var i=0; i<sliceLabelData.length; ++i)
                            sliceLabels.push_back(sliceLabelData[i]);
                        ravel.addHandle(this.axis,sliceLabels);
                        if (ravel.numHandles()>=2)
                            ravel.draw();
                    }
                }
                // request slicelabels
                sliceLabelReq.open("GET","/mySqlService.php/axes/"+table+"/"+axes[i]);
                sliceLabelReq.send();
            }
        }
    }
    // request axis names
    xhttp.open("GET","/mySqlService.php/axes/"+table);
    xhttp.send();
}

function onunload() {
    if (ravel) ravel.delete();
}

