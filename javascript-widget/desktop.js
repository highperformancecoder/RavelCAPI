var selector=document.getElementById("tableSelector1lhs");
var openFile=document.createElement("option");
selector.appendChild(openFile);
openFile.innerHTML="Open CSV file";
selector.setAttribute("onchange","openFile(ravel1.lhs)");

const dialog = require('electron').dialog;

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
function openFile(ravel)
{
    console.log("in openFile");
    dialog.showOpenDialog({}, function (file) {
        alert(file);
    });
}
