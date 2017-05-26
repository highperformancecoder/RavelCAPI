/*
  © Ravelation Pty Ltd 2017
  Example code released under the MIT license.
*/

// a difference function for use in custom JS op.
var diff=function(x)
{
    var r=x-diff.xsaved;
    diff.xsaved=x;
    if (r===NaN)
        return 0; // probably start of series
    else
        return r;
}

function handleOpSelector(ravel)
{
    var id = ravel==ravel1? "op1": "op2";
    switch (document.getElementById(id).value)
    {
        case '+': ravel.dataFunctor=function(x,y) {return x+y;}; break;
        case '-': ravel.dataFunctor=function(x,y) {return x-y;}; break;
        case '*': ravel.dataFunctor=function(x,y) {return x*y;}; break;
        case '/': ravel.dataFunctor=function(x,y) {return x/y;}; break;
        case 'custom':
        ravel.functorCode=prompt
             ('Please enter a custom Javascript function to combine values. Standard mathematical functions are available in the Math namespace, eg Math.log(). You may also use diff(x) which returns the difference with respect to the previous value. If you are processing only a single dataset, please use a single argument function, eg function(x) {return diff(x);}',
              ravel.functorCode);
        eval("ravel.dataFunctor="+ravel.functorCode);
        break;
    }
    plotAllData();
}

function makeRadio(row, ravel, axis, radioName, checkedName, selections)
{
    var radioBox=document.createElement("td");
    row.appendChild(radioBox);
    radioBox.setAttribute("style","border:1px solid black");
    for (var i=0; i<selections.length; ++i)
    {
        var item=document.createElement("div");
        radioBox.appendChild(item);
        item.setAttribute("class","tooltip");
        var input=document.createElement("input");
        item.appendChild(input);
        input.setAttribute("type","radio");
        input.setAttribute("name",axis+"-"+radioName);
        input.setAttribute("value",selections[i].value);
        if (selections[i].value===checkedName)
        {
            input.setAttribute("checked","1");
        }
        input.ravel=ravel;
        input.axis=axis;
        input.setAttribute("onchange",'radio'+radioName+'Pushed(this)');
        var span=document.createElement("span");
        item.appendChild(span);
        span.setAttribute("class","tooltiptext");
        span.innerHTML=selections[i].tooltiptext;
    }
}

function makeSelect(row, ravel, axis, radioName, checkedName, selections)
{
    var item=document.createElement("td");
    row.appendChild(item);
//    radioBox.setAttribute("style","border:1px solid black");
    var select=document.createElement("select");
    item.appendChild(select);
    for (var i=0; i<selections.length; ++i)
    {
        var option=document.createElement("option");
        select.appendChild(option);
        option.innerHTML=selections[i].tooltiptext;
        option.setAttribute("value",selections[i].value);
        if (selections[i].value===checkedName)
        {
            option.setAttribute("selected","1");
        }
        select.ravel=ravel;
        select.axis=axis;
        select.setAttribute("onchange",'radio'+radioName+'Pushed(this)');
    }
}

function radiosortPushed(input)
{
    var h=input.ravel.handle;
    h.get(input.axis);
    h.sliceLabels.setOrder(Module.Order[input.value]);
    plotAllData();
    input.ravel.redraw();
}

function radioreducePushed(input)
{
    var h=input.ravel.handle;
    h.get(input.axis);
    h.setReductionOp(Module.ReductionOp[input.value]);
    plotAllData();
    input.ravel.redraw();
}

function hideAxisMenus(menuID) {
    var menu=document.getElementById(menuID);
    while (menu.hasChildNodes())
        menu.removeChild(menu.firstChild);
}

function toggleAxisMenus(menuID, ravel) {
    var menu=document.getElementById(menuID);
    if (menu.hasChildNodes())
    {
        while (menu.hasChildNodes())
            menu.removeChild(menu.firstChild);
        return;
    }

    var header=document.createElement("tr");
    menu.appendChild(header);
    var item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Axis";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Reduce";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Sort";
    item=document.createElement("th");
    header.appendChild(item);
    item.innerHTML="Filtering";

    
    for (var i=0; i<ravel.numHandles(); ++i)
    {
        var h=ravel.handle;
        h.get(i);
        var row=document.createElement("tr");
        menu.appendChild(row);
        var item=document.createElement("td");
        row.appendChild(item);
        item.innerHTML=h.getDescription();

        var reductionSelector=
            [
                {"value": "sum", "tooltiptext": "sum"},
                {"value": "prod", "tooltiptext": "product"},
                {"value": "av", "tooltiptext": "average"},
                {"value": "stddev", "tooltiptext": "standard deviation"},
                {"value": "min", "tooltiptext": "minimum"},
                {"value": "max", "tooltiptext": "maximum"},
            ];
        makeSelect(row, ravel, i, "reduce", "sum", reductionSelector);
        var sortSelector=
            [
                {"value":"none", "tooltiptext": "None"},
                {"value":"forward", "tooltiptext": "Forward"},
                {"value":"reverse", "tooltiptext": "Reverse"},
                {"value":"numForward", "tooltiptext": "Numerically Forward"},
                {"value":"numReverse", "tooltiptext": "Numerically Reverse"}
            ];
        makeSelect(row, ravel, i, "sort", "forward", sortSelector);

        item=document.createElement("td");
        row.appendChild(item);
        var input=document.createElement("input");
        item.appendChild(input);
        input.setAttribute("type","checkbox");
        input.setAttribute("name","filter");
        input.setAttribute("onchange","toggleFilter(this)");
        input.axis=i;
        input.ravel=ravel;
    }
}

function toggleFilter(checkBox)
{
    var h=checkBox.ravel.handle;
    h.get(checkBox.axis);
    h.setDisplayFilterCaliper(checkBox.checked);
    checkBox.ravel.redraw();
}

function Ravel1D(canvas) {
    var master=this.master=newRavel(canvas);
    var lhs=this.lhs=newRavel("hiddenCanvas");
    var rhs=this.rhs=newRavel("hiddenCanvas");
    master.setRank(1);
    lhs.setRank(1);
    rhs.setRank(1);
    this.delete=function() {
        this.lhs.delete();
        this.rhs.delete();
        this.master.delete();
    }
    var synchroniseMaster = function() {
        var axisData={};
        function addAxisDataFrom(ravel) {
            for (var i=0; i<ravel.numHandles(); ++i)
            {
                var h=ravel.handle;
                h.get(i);
                var ad=axisData[h.getDescription()]=[];
                for (var j=0; j<h.sliceLabels.size(); ++j)
                {
                    ad.push(h.sliceLabelAt(j));
                }
            }
        }
        addAxisDataFrom(lhs);
        addAxisDataFrom(rhs);
        master.clear();
        for (i in axisData)
        {
            master.addHandle(i,axisData[i]);
        }
        makeCountryDefaultX(master);
        plotAllData();
    }
    lhs.dataLoadHook=synchroniseMaster;
    rhs.dataLoadHook=synchroniseMaster;
    master.onRedraw=function() {plotMasterData(master);}
    this.functorCode="function(x,y) {return x/y;};";
    this.dataFunctor=function(x,y) {return x/y;};
}

Ravel1D.prototype=new Object;

// make country the default x-axis
function makeCountryDefaultX(ravel) {
    for (var i=0; i<ravel.numHandles(); ++i) {
        var h=ravel.handle;
        h.get(i);
        // select reverse sorting
        h.sliceLabels.setOrder(Module.Order.forward);

        if (h.getDescription()==="country") {
            ravel.setHandleIds([i]);
            ravel.redistributeHandles();
        } else {
            h.setSliceIndex(h.sliceLabels.size()-1);
        }
    }
    ravel.redraw();
}
    
// align handlers and slicers of slave ravel to that of master
function alignHandles(master,slave) {
    if (slave.numHandles()>slave.handleId(0))
    {
        var mh=master.handle;
        var sh=slave.handle;
        
        mh.get(master.handleId(0));
        var xDesc=mh.getDescription();
        var xOp=mh.getReductionOp();
        var xOrder=mh.sliceLabels.getOrder();
        // didn't combine, try rotating slave ravel to match orientation of master
        for (var i=0; i<slave.numHandles(); ++i)
        {
            sh.get(i);
            if (sh.getDescription()===xDesc)
            {
                if (slave.getHandleIds()[0]!=i)
                {
                    slave.setHandleIds([i]);
                    slave.redistributeHandles();
                }
                sh.sliceLabels.setOrder(xOrder);
                sh.setReductionOp(xOp);
            }
            else // check that slicers match
            {
                for (var j=0; j<master.numHandles(); ++j)
                {
                    mh.get(j);
                    if (sh.getDescription()===mh.getDescription())
                    {
                        sh.setSlicer(mh.sliceLabel());
                        if (sh.collapsed()!=mh.collapsed())
                        {
                            sh.toggleCollapsed();
                        }
                        sh.sliceLabels.setOrder(mh.sliceLabels.getOrder());
                        sh.setReductionOp(mh.getReductionOp());
                    }
                }
            }
        }
        slave.redraw();
    }
}

function plotData(ravel) {
    var plotlyData={
        name: "",
        type: document.getElementById("plotType").value,
        mode: 'lines',
        x: [], y: [],
        xlabels: []
    };

    if (ravel.master.numHandles()<=ravel.master.handleId(0)) return plotlyData; //nothing to do
    var xh=ravel.master.handle;
    xh.get(ravel.master.handleId(0));
    plotlyData.name=ravel.lhs.table;

    if (xh.collapsed())
    {
        ravel.lhs.toggleCollapsed(ravel.lhs.handleId(0));
        ravel.rhs.toggleCollapsed(ravel.rhs.handleId(0));
    }
    
    // arithmetic operation extracted out, as we may need to try this twice
    function tryCombine()
    {
        ravel.lhs.hyperSlice();
        ravel.rhs.hyperSlice();
        var slice1=ravel.lhs.output;
        var slice2=ravel.rhs.output;
        
        var xh=ravel.lhs.handle;
        xh.get(ravel.lhs.handleId(0));

        diff.xsaved=NaN; // reset diff saved value in case it is used
        
        for (var i=0; i<xh.sliceLabels.size(); ++i)
        {
            var key=[{axis:xh.getDescription(), slice:xh.sliceLabelAt(i)}];
            var value=slice1.val(key);
            if (isFinite(value))
            {
                if (slice2.rank()>0 && ravel.rhs.numHandles()>ravel.rhs.handleId(0))
                {
                    var h2=ravel.rhs.handle;
                    h2.get(ravel.rhs.handleId(0));
                    key[0].axis=h2.getDescription();
                    value2=slice2.val(key);
                    value=ravel.dataFunctor(value,value2);
                }
                else if (ravel.dataFunctor.length===1)
                    value=ravel.dataFunctor(value);
            }
            if (isFinite(value) || document.getElementById("allDomain").checked)
            {
                plotlyData.x.push(i);
                if (isFinite(value))
                    plotlyData.y.push(value);
                else
                    plotlyData.y.push(NaN);
            }
            plotlyData.xlabels.push(xh.sliceLabelAt(i));
        }
        
        if (plotlyData.x.length==0)
        {
            // no second ravel data matched, so replace by that of ravel 1
            for (var i=0; i<xh.sliceLabels.size(); ++i)
            {
                var key=[{axis:xh.getDescription(), slice:xh.sliceLabelAt(i)}];
                var value=slice1.val(key);
                if (isFinite(value))
                {
                    plotlyData.x.push(xh.sliceLabelAt(i));
                    plotlyData.y.push(value);    
                }
            }
        }
                
    }

    alignHandles(ravel.master,ravel.lhs);
    alignHandles(ravel.master,ravel.rhs);
    tryCombine();
   
    return plotlyData;
};

function processData(ravel) {/* not used */}
function plotAllData() {plotMasterData(ravel1.master);}

function plotMasterData(masterRavel) {
    if (document.getElementById("slaved").checked)
    {
        if (masterRavel==ravel1.master)
            alignHandles(ravel1.master, ravel2.master);
        else
            alignHandles(ravel2.master, ravel1.master);            
    }
    var data=[plotData(ravel1), plotData(ravel2)];
    data[1].yaxis="y2";

    var minx=data[0].x[0];
    if (data[1].x[0]<minx) minx=data[1].x[0];
    var maxx=data[0].x[data[0].x.length-1];
    if (data[1].x[data[0].x.length-1]>maxx) maxx=data[1].x[data[0].x.length-1];
    var xticks=[], xlabels=[];
    var incr=Math.floor((maxx-minx)/30);
    if (incr==0) incr=1;
    for (var i=minx; i<=maxx; i+=incr)
    {
        xticks.push(i);
        xlabels.push(data[0].xlabels[i]);
    }
    
    var layout = {
        title: "By "+ravel1.master.description(),
        xaxis: {
            showgrid: false,
            zeroline: false,
            tickmode: "array",
            tickvals: xticks,
            ticktext: xlabels
        },
        yaxis: {
            title: data[0].name,
            showline: false,
        },
        yaxis2: {
            title: data[1].name,
            overlaying: "y",
            side: "right"
        }
    };

    var xh=ravel1.master.handle;
    xh.get(ravel1.master.handleId(0));
    layout.xaxis.title=xh.description;

    // allow manual ranges to be set
    var y1min=document.getElementById("y1min").value;
    var y1max=document.getElementById("y1max").value;
    var y2min=document.getElementById("y2min").value;
    var y2max=document.getElementById("y2max").value;
    if (y1min!="" && isFinite(y1min) && y1max!="" && isFinite(y1max))
    {layout.yaxis.range=[y1min,y1max];}
    else
    {delete layout.yaxis.range;}
    if (y2min!="" && isFinite(y2min) && y2max!="" && isFinite(y2max))
    {layout.yaxis2.range=[y2min,y2max];}
    else
    {delete layout.yaxis2.range;}

    // if no data, and no range set, set to [0,1]
    if (data[0].x.length==0 && layout.yaxis.range==undefined)
        layout.yaxis.range=[0,1];
    if (data[1].x.length==0 && layout.yaxis2.range==undefined)
        layout.yaxis2.range=[0,1];
        
    // if no y2 data, remove anything related to it
    if (data[1].y.length==0)
    {
        data.length=1;
        delete layout.yaxis2;
    }
    
    var plot=document.getElementById("plot");
    Plotly.purge(plot);
    Plotly.plot(plot,data,layout);
    // for debugging memory leak problems caused by lack of finalisers
    //    in javascript
    // console.log("mem usage="+DYNAMICTOP);
}
