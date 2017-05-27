var ids=["tableSelector1lhsWrap","tableSelector1rhsWrap","tableSelector2lhsWrap","tableSelector2rhsWrap"];
for (var i in ids)
{
    var selector=document.getElementById(ids[i]);
    while (selector.hasChildNodes()) selector.removeChild(selector.firstChild);
    var openFile=document.createElement("input");
    selector.appendChild(openFile);
    openFile.setAttribute("type","button");
    openFile.setAttribute("value","Open CSV file");
    selector.setAttribute("onclick","openFile(id,ravel1.lhs)");
}

const {dialog} = require('electron').remote;

fs=require('fs');
var fileSplit=/(.*)\/([^/]*)$/;

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
global.openFile=function (id,ravel)
{
    console.log("in openFile");
    dialog.showOpenDialog({}, function (file) {
        var filePath=fileSplit.exec(file);
        document.getElementById(id).firstChild.setAttribute("value",filePath[2]);

        var data=JSON.parse(fs.readFileSync(file[0],{encoding: 'utf8'}));
        //        var data=eval(fs.readFileSync(file[0],{encoding: 'utf8'}));
        ravel.clear();
        for (var i=0; i<data.dimensions.length; ++i)
            ravel.addHandle(data.dimensions[i].axis,data.dimensions[i].slice);
        var axes=[];
        for (var i=0; i<data.dimensions.length; ++i) axes.push(data.dimensions[i].axis);
        ravel.dimension(axes);
        ravel.loadData(data.data);
        ravel.dataLoadHook();
    });
}
