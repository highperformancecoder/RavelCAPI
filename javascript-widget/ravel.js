function findPos(obj) {
    var curleft = curtop = 0;
    if (obj.offsetParent) {
        do {
	    curleft += obj.offsetLeft;
	    curtop += obj.offsetTop;
        } while (obj = obj.offsetParent);
    }
    return [curleft,curtop];
}

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
    var adjustOrigin=function() {
        var offsets=findPos(canvasElem);
        ravel.x=0.5*canvasElem.width+offsets[0];
        ravel.y=0.5*canvasElem.height+offsets[1];
    }
    var x=function(event) {
        return event.clientX+window.pageXOffset;
    }
    var y=function(event) {
        return event.clientY+window.pageYOffset;
    }
    canvasElem.onmousedown=function(event) {
        adjustOrigin();
        ravel.onMouseDown(x(event), y(event));
    };
    canvasElem.onmouseup=function(event) {
        adjustOrigin();
        ravel.onMouseUp(x(event), y(event));
        ravel.redraw();
        ravel.onRedraw();
    };
    canvasElem.onmousemove=function(event) {
        adjustOrigin();
        if (ravel.onMouseOver(x(event), y(event)))
            ravel.redraw();
        if (event.button==0 && ravel.onMouseMotion(x(event), y(event)))
            ravel.redraw();
    };
//    // these two are needed to capture keyboard focus when mouse hovering over ravel
    canvasElem.onmouseover=function(event) {
        canvasElem.focus();
    };
    canvasElem.onmouseout=function(event) {
        canvasElem.blur();
    };
    canvasElem.onkeydown=function(event) {
        switch (event.key)
        {
            case "ArrowLeft": ravel.handleLeftKey(); break;
            case "ArrowRight": ravel.handleRightKey(); break;
        }
        ravel.redraw();
        ravel.onRedraw();
        event.preventDefault();
    }
    canvasElem.onkeyup=function(event) {event.preventDefault();}
    canvasElem.onkeypress=function(event) {event.preventDefault();}
    canvasElem.onwheel=function(event) {
        if (event.deltaY)
        {
            if (event.deltaY>0)
                ravel.handleLeftKey();
            else 
                ravel.handleRightKey();
            ravel.redraw();
            ravel.onRedraw();
        }
        event.preventDefault();
        event.stopPropagation();
    }
    
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
    var r="/mySqlService.php/data/"+ravel.table+"?";
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
    var r="/mySqlService.php/data/"+ravel.table+"?";
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        if (i>0) r+="&";
        var h=ravel.handles(i);
        r+=h.description+"=";
    }
    return r;
}

// populate table selector
function populateTableSelector(selectorId)
{
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            var response = eval(this.responseText);
            response.unshift("-"); // a dummy table name to indicate unselected
            var tableSelector=document.getElementById(selectorId);
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
}

function setTable(name,ravel) {
    var xhttp = new XMLHttpRequest();
    ravel.table=name;
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
                            ravel.dimension(axes);
                            ravel.redraw();

                            var dataReq=new XMLHttpRequest;
                            var dbQuery="/mySqlService.php/allData/"+ravel.table;
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
                sliceLabelReq.open("GET","/mySqlService.php/axes/"+ravel.table+"/"+axes[i]);
                sliceLabelReq.send();
            }
        }
    }
    // request axis names
    xhttp.open("GET","/mySqlService.php/axes/"+ravel.table);
    xhttp.send();
}

function addCommand(menu,label,cmd)
{
    var entry=document.createElement("li");
    menu.appendChild(entry);
    entry.innerHTML=h.description;
    entry.setAttribute("onclick",cmd);
}

function addMenu(menu,label)
{
    var entry=document.createElement("li");
    menu.appendChild(entry);
    var a=document.createElement("a");
    entry.appendChild(a);
    a.innerHTML=label;
    a.setAttribute("href","#");
    a.setAttribute("class","hsub");
    var subMenu=document.createElement("ul");
    entry.appendChild(subMenu);
    return subMenu;
}

function axisMenu(ravel, menuId)
{
    var menu=document.getElementById(menuId);
    while (menu.hasChildNodes())
    {menu.removeChild(menu.firstChild);}
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        var h=ravel.handles(i);
        var submenu=addMenu(menu,h.description);
        addMenu(submenu,"sort");
        addMenu(submenu,"reduce");
        addMenu(submenu,"filter");
        h.delete;
    }
}
