var ids=["tableSelector1lhsWrap","tableSelector1rhsWrap","tableSelector2lhsWrap","tableSelector2rhsWrap"];
var ravels=[ravel1.lhs,ravel1.rhs,ravel2.lhs,ravel2.rhs];
for (var i in ids)
{
    var selector=document.getElementById(ids[i]);
    while (selector.hasChildNodes()) selector.removeChild(selector.firstChild);
    var openFile=document.createElement("input");
    selector.appendChild(openFile);
    openFile.setAttribute("type","button");
    openFile.setAttribute("value","Open file");
    selector.setAttribute("onclick","openFile("+i+")");
}

const {dialog} = require('electron').remote;

fs=require('fs');
var fileSplit=/(.*)[\\\/]([^/]*)$/;

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
global.openFile=function (idx)
{
    dialog.showOpenDialog({}, function (file) {
        var filePath=fileSplit.exec(file);
        document.getElementById(ids[idx]).firstChild.setAttribute("value",filePath[2]);

        var data=JSON.parse(fs.readFileSync(file[0],{encoding: 'utf8'}));
        ravels[idx].clear();
        for (var i=0; i<data.dimensions.length; ++i)
            ravels[idx].addHandle(data.dimensions[i].axis,data.dimensions[i].slice);
        var axes=[];
        for (var i=0; i<data.dimensions.length; ++i) axes.push(data.dimensions[i].axis);
        ravels[idx].dimension(axes);
        ravels[idx].loadData(data.data);
        ravels[idx].dataLoadHook();
    });
}
