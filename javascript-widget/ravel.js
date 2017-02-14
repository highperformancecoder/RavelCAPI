var table=""; // selected table

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
    var tableBlock=document.getElementById("tableBlock");
    ravel.x=0.5*canvasElem.width+tableBlock.offsetLeft;
    ravel.y=0.5*canvasElem.height+tableBlock.offsetTop;
    canvasElem.onmousedown=function(event) {
        ravel.onMouseDown(event.clientX, event.clientY);
    };
    canvasElem.onmouseup=function(event) {
        ravel.onMouseUp(event.clientX, event.clientY);
        ravel.redraw();
        ravel.onRedraw();
    };
    canvasElem.onmousemove=function(event) {
        if (ravel.onMouseOver(event.clientX, event.clientY))
            ravel.redraw();
        if (event.button==0 && ravel.onMouseMotion(event.clientX, event.clientY))
            ravel.redraw();
    };
//    canvasElem.ondblclick=function(event) {
//        var h=ravel.handleIfMouseOver(event.clientX, event.clientY, -1);
//        if (h>=0)
//        {
//            ravel.handles(h).toggleCollapsed();
//            ravel.redraw();
//            ravel.onRedraw();
//        }
//    }
    
    return ravel;
}

var buildDbQuery=function(db,ravel) {
    var r="/mySqlService.php/data/"+table+"?";
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        if (i>0) r+="&";
        var h=ravel.handles(i);
        r+=h.description+"=";
        if (i!=ravel.xHandleId() && i!=ravel.yHandleId())
        {
            if (h.collapsed())
                r+="reduce("+reductionOp+")";
            else
                r+="slice("+h.sliceLabel()+")";
        }
    }
    return r;
}

/// obtain the full database, regardless of ravel configuration
var fullDbQuery=function(db,ravel) {
    var r="/mySqlService.php/data/"+table+"?";
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        if (i>0) r+="&";
        var h=ravel.handles(i);
        r+=h.description+"=";
    }
    return r;
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

function setTable(name) {
    table=name;
    ravel.clear();
    
    ravel.onRedraw=function() {processData(ravel);}

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
                        if (ravel.numHandles()==axes.length)
                        {
                            ravel.redraw();
                            ravel.dimension(axes);
                            var dataReq=new XMLHttpRequest;
                            var dbQuery="/mySqlService.php/allData/"+table;
                            dataReq.onreadystatechange = function() {
                                if (this.readyState == 4 && this.status == 200) {
                                    ravel.loadData(this.responseText);
                                    ravel.onRedraw();
                                }
                            }
                            dataReq.open("GET",dbQuery);
                            dataReq.send();
                        }
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

