var table=""; // selected table

function sqr(x) {return x*x;};
var palette=["black","red","green","blue","magenta","cyan","yellow"];

var newRavel = function(canvasId) {
    var ravel = new Module.RavelCairo;
    var canvasElem=document.getElementById(canvasId);
    var canvas = canvasElem.getContext('2d');
    var radius = 0.5*Math.min(canvasElem.width,canvasElem.height);
    ravel.setCanvas(canvas);
    ravel.rescale(0.8*radius);
    ravel.x=0; ravel.y=0;
    canvas.translate(0.5*canvasElem.width, 0.5*canvasElem.height);
    ravel.redraw = function() {
        canvas.clearRect(-0.5*canvasElem.width, -0.5*canvasElem.height,
                         canvasElem.width, canvasElem.height);
        ravel.render();
    }
   
    // bind mouse actions
    var offx=canvasElem.parentElement.offsetLeft+100;
    var offy=canvasElem.parentElement.offsetTop+100;
    ravel.x=0; ravel.y=0;
    canvasElem.onmousedown=function(event) {
        ravel.onMouseDown(event.clientX-offx, event.clientY-offy);
    };
    canvasElem.onmouseup=function(event) {
        ravel.onMouseUp(event.clientX-offx, event.clientY-offy);
        ravel.redraw();
    };
    canvasElem.onmousemove=function(event) {
        if (ravel.onMouseOver(event.clientX-offx, event.clientY-offy))
            ravel.redraw();
        if (event.button==0 && ravel.onMouseMotion(event.clientX-offx, event.clientY-offy))
            ravel.redraw();
    };
    canvasElem.ondblclick=function(event) {
        var h=ravel.handleIfMouseOver(event.clientX-offx, event.clientY-offy, -1);
        alert(h);
        if (h>=0)
        {
            ravel.handles(h).toggleCollapsed();
            ravel.redraw();
        }
    }
    
    return ravel;
}

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

var ravel=newRavel("ravel");

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
                            ravel.redraw();
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

