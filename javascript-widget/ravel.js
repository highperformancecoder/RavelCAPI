var table=""; // selected table

function sqr(x) {return x*x;};
var palette=["black","red","green","blue","magenta","cyan","yellow"];

/*
  HTMLCanvas is an array of HTML canvas contexts. Add a new context
  using the method newCanvas. Returns index of new context.
*/
var HTMLcanvas = [];
HTMLcanvas.newCanvas = function(id) {
    alert("id="+id);
    var canvas = document.getElementById(id);
    if (canvas.getContext) {
        HTMLcanvas.push(canvas.getContext('2d'));
    }
    return HTMLcanvas.length-1;
}



var JSRavel = Module.JRavelCairo.extend("JRavelCairo", {
    // container of references to SVG objects
    __construct: function(svgFrame) {
        this.svgFrame=svgFrame;
        this.handle=new Array;
        this.__parent.__construct.call();
        for (var i in this)
            document.writeln(i);

        var canvas = document.getElementById(svgFrame);
        alert(svgFrame);
        this.setCanvas(canvas.getContext('2d'));
        this.rescale(140);
        alert(svgFrame);

         // bind mouse actions
        var ravelframe=document.getElementById(svgFrame);
        var elem=ravelframe.parentElement;
        var offx=elem.parentElement.offsetLeft+100;
        var offy=elem.parentElement.offsetTop+100;
        this.x=0; this.y=0;
        alert(svgFrame);
        var ravel=this;
       elem.onmousedown=function(event) {
            ravel.onMouseDown(event.clientX-offx, event.clientY-offy);
        };
        elem.onmouseup=function(event) {
            ravel.onMouseUp(event.clientX-offx, event.clientY-offy);
            ravel.render();
        };
        elem.onmousemove=function(event) {
            if (ravel.onMouseMotion(event.clientX-offx, event.clientY-offy))
                ravel.render();
        };
    },
});

//var JSRavel = function(svgFrame) {
//    var ravel = new Module.JRavelCairo(svgFrame);
//    this.ravel=ravel; //TODO necessary?
//    ravel.rescale(140);
//             // bind mouse actions
//    var ravelframe=document.getElementById(svgFrame);
//    var elem=ravelframe.parentElement;
//    var offx=elem.parentElement.offsetLeft+100;
//    var offy=elem.parentElement.offsetTop+100;
//    ravel.x=0; ravel.y=0;
//    elem.onmousedown=function(event) {
//        ravel.onMouseDown(event.clientX-offx, event.clientY-offy);
//    };
//    elem.onmouseup=function(event) {
//        ravel.onMouseUp(event.clientX-offx, event.clientY-offy);
//        ravel.render();
//    };
//    elem.onmousemove=function(event) {
//        if (ravel.onMouseMotion(event.clientX-offx, event.clientY-offy))
//            ravel.render();
//    };
//}

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

//var ravel=new JSRavel("ravel");
var ravel=new Module.JRavelCairo;
var canvas = document.getElementById("ravel").getContext('2d');
ravel.setCanvas(canvas);
ravel.rescale(140);
ravel.x=0; ravel.y=0;
canvas.translate(150,150);

//var tm=canvas.getContext('2d').measureText("hello");
//for (var i in tm) document.writeln(i);

function setTable(name) {
    table=name;
    //ravel.clearHandles();
    
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
                            ravel.render();
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

