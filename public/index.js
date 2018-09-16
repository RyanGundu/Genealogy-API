// Put all onload AJAX calls here, and event listeners

var AllFiles = [];
var AllIndividuals = [];
var AllFileData = [];
var completedFiles = [];
var filesInDB = [];


$(document).ready(function() {
    // On page-load AJAX Example
    onLoad();
    dataLogIn();

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
    });

});



function onLoad () {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/fileNames',   //The server endpoint we are connecting to
        success: function (data) {
            for (let i = 0; i < data.length; i++) { 
                if (data[i].indexOf('.ged') != -1) {
                    loadFiles(data[i]);
                    updateListSelect(data[i]);
                    if (AllFiles.indexOf(data[i]) == -1)
                        AllFiles.push(data[i]);
                    loadIndividuals(data[i]);
                }
            }

            if (AllFiles.length == 0) {
                $('.indDisplay').html('NO FILES');
                $('.fileLog').html('NO FILES');
            }
           // console.log(data);
            //We write the object to the console to show that the request was successful
            //console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/FileLog',   //The server endpoint we are connecting to
        success: function (data) {
            for (let i = 0; i < data.length; i++) { 
                loadLog(JSON.parse(data[i]), i+1);
            }
            //console.log(data);
            //We write the object to the console to show that the request was successful 
        },
        fail: function(error) {
            console.log(error); 
        }
    });

}

function dataLogIn() {

    $(':button').prop('disabled', true); // Disable all the buttons
    document.getElementById("conDB").disabled = false;

    let modal = document.getElementById('popUp');
    modal.style.display = "block";

    let con = document.getElementById("conDB");
    con.onclick = function() {

        let host =  document.getElementsByName("host")[0].value;
        let user =  document.getElementsByName("user")[0].value;
        let pw =  document.getElementsByName("password")[0].value;
        let db =  document.getElementsByName("db")[0].value;

        createConnectionToDB(host, user, pw, db);
        // modal.style.display = "none";
    }

}

function createConnectionToDB(host, user, pw, db) {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/connectDB?host='+ host +'&user=' + user + '&pw=' + pw + '&db=' +db,   //The server endpoint we are connecting to
        success: function (data) {
            //console.log(data);
            if (data[0] == "ERROR") {
                let err = document.getElementById('connectionStatus');
                err.style.display = "block";
            } else {
                createTables();
                let modal = document.getElementById('popUp');
                $(':button').prop('disabled', false); 
                modal.style.display = "none";
            }
            onLoad();
            //We write the object to the console to show that the request was successful 
        },
        fail: function(error) {
            console.log(error); 
        }
    });

}

function createTables() {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/createTables',   //The server endpoint we are connecting to
        success: function (data) {
            //console.log(data);
            //We write the object to the console to show that the request was successful 
        },
        fail: function(error) {
            console.log(error); 
        }
    });
}


/****************************** MY CODE **********************/

function clearStatus() {
    $('#statBar').html('<p>Status: </p>');
    let panel = document.getElementById('statBar');
    panel.scrollTop = panel.scrollHeight - panel.scrollHeight;
}

function updateStatus(string) {

    $('#statBar').append('<p>Status: '+string+'</p>');
    //Keep status bar at bottom
    let panel = document.getElementById('statBar');
    panel.scrollTop = panel.scrollHeight;

}

///// Function to load lists of new files (downloadable)////
function loadFiles (string) {

    updateFiles("<a class=\"dropdown-item\" href=\"uploads/"+ string + "\"" + ">" + string + "</a>", string);
}

//Uplaods lists in dropdown menu (downloadable)
function updateFiles(newHTML, fName) {
    let list = document.getElementById('filesList').innerHTML;
    if (list.indexOf(newHTML) == -1) {
        if (list.indexOf("<a class=\"dropdown-item\">Currently No files</a>") != -1) {
            list = newHTML;
        } else {
            list += newHTML;
        }
        document.getElementById('filesList').innerHTML = list;

        let fLog = document.getElementById('fLog').innerHTML;
        if (fLog == 'NO FILES') {
            document.getElementById('fLog').innerHTML = '<tr class="stick"><th>File Name<br>(Click to download)</th><th>Source</th><th>GEDC<br>version</th><th>Encoding</th><th>Submitter<br>name</th><th>Submitter<br>Address</th><th>Number<br>of<br>individuals</th><th>Number<br>of<br>families</th></tr>';
        }
        $('.fileLog').append('<tr id="'+ fName +'"><td><a>' + fName + '</a></td></tr>');

    }
    
}

//Uplaods lists in dropdown menu (non-downloadable)
function updateListSelect(fName) {

    let list = "<a class=\"dropdown-item\" onclick=\"PVFileName('"+ fName + "')\">" + fName + "</a>";
    let list1 = "<a class=\"dropdown-item\" onclick=\"ADDFileName('"+ fName + "')\">" + fName + "</a>";
    let list2 = "<a class=\"dropdown-item\" onclick=\"DECFileName('"+ fName + "')\">" + fName + "</a>";
    let list3 = "<a class=\"dropdown-item\" onclick=\"ANCFileName('"+ fName + "')\">" + fName + "</a>";

    let toCheck = document.getElementById('listSelect').innerHTML;
    if (toCheck.indexOf(fName) == -1) {
        document.getElementById('listSelect').innerHTML += list;
        document.getElementById('listSelect1').innerHTML += list1;
        document.getElementById('listSelect2').innerHTML += list2;
        document.getElementById('listSelect3').innerHTML += list3;
    }

}
//////////////// Done updating file lists //////////////

////////// IF THEY SELECT A FILE ///////
function PVFileName(fName) {

    document.getElementById('VPFile').innerHTML = "File Selected: " + fName;
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/DisplayData?fName=' + fName,   //The server endpoint we are connecting to
        success: function (data) {

            if (data.length == 0) {
                $('.indDisplay').html('NO INDIVIDUALS');
                return;
            } else if (data[0] == "NI") {
                $('.indDisplay').html('NO INDIVIDUALS');
                return;
            }

            $('.indDisplay').html('<tr class="rowFade"><th>Firstname</th><th>Lastname</th><th>Sex</th><th>Family Size</th></tr>');
            for (var i = 0; i < data.length; i++) {
                loadDisplay(data[i], i+1, fName);
            }

            //console.log(data);
        },
        fail: function(error) {
            console.log(error);
        }
    });
    
}


function ADDFileName(fName) {

    document.getElementById('ADDFile').innerHTML = "File Selected: " + fName;

}
function DECFileName(fName) {

    document.getElementById('DECFile').innerHTML = "File Selected: " + fName;
}
function ANCFileName(fName) {

    document.getElementById('ANCFile').innerHTML = "File Selected: " + fName;
}

/******************************************************/

// LOADING DISPLAYS
function loadDisplay (ind, rowNum, fileName) {
    
    let style = '<tr>';
    if (rowNum%2 == 0)
        style = '<tr class="rowFade">';

    if (ind.famSize == 0)
        ind.famSize = 1;

    $('.indDisplay').append(style + '<td>' + ind.givenName +'</td><td>' + ind.surname + '</td><td>'+ ind.sex +'</td><td>' + ind.famSize + '</td></tr>');
}


//LOAD FILE LOG
function loadLog (fileObj, rowNum) {

    let fileInfo = new Object();
    fileInfo.fileName = fileObj.fileName;
    fileInfo.source = fileObj.source;
    fileInfo.version = fileObj.gedcVersion;
    fileInfo.encoding = fileObj.encoding;
    fileInfo.subName = fileObj.subName;
    fileInfo.subAddr = fileObj.subAddress;
    fileInfo.numInd = fileObj.numInd;
    fileInfo.numFam = fileObj.numFam;

    let duplicate = false;
    for (var i = 0; i < AllFileData.length; i++) {
        if (AllFileData[i].fileName == fileInfo.fileName)
            duplicate = true;
    }

    if (!duplicate)
        AllFileData.push(fileInfo);

    let style = '<td>';
    if (rowNum%2 == 0) {
        style = '<td class="rowFade">';
    }

    let log = style + '<a href="uploads/'+ fileObj.fileName +'">'+ fileObj.fileName +'</a></td>' + style + fileObj.source +'</td>' + style + fileObj.gedcVersion + '</td>' + style + fileObj.encoding + '</td>';
    log += style + fileObj.subName +'</td>' + style + fileObj.subAddress + '</td>' + style + fileObj.numInd + '</td>' + style + fileObj.numFam + '</td>';
    document.getElementById(fileObj.fileName).innerHTML = log;
}

function loadIndividuals (fName) {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/DisplayData?fName=' + fName,   //The server endpoint we are connecting to
        success: function (data) {

            if (data.length == 0) {
                return;
            } else if (data[0] == "NI") {
                return;
            }
            for (var i = 0; i < data.length; i++) {
                splitInd(data[i], fName);
            }
            if (completedFiles.indexOf(fName) == -1)
                completedFiles.push(fName);

            //console.log(data);
        },
        fail: function(error) {
            console.log(error);
        }
    });

}

function splitInd (ind, fName) {

    if (ind.famSize == 0)
        ind.famSize = 1;

    if (completedFiles.indexOf(fName) != -1) {
        return;
    }

    let individual = new Object();
    individual.fileName = fName;
    individual.surname = ind.surname;
    individual.givenName = ind.givenName;
    individual.sex = ind.sex;
    individual.famSize = ind.famSize;

    AllIndividuals.push(individual);

}

/************ Take File To Upload *************/
function validate() {

    let fptr = document.getElementById('myFile');

    if (fptr == '') {
        console.log('INVALID');
        updateStatus('No file selected');
        return false;
    }

    if (fptr.files.length == 0) 
            return false;

    for (var i = 0; i < AllFiles.length; i++) {
        if (AllFiles[i] == fptr.files[0].name) {
            updateStatus(AllFiles[i] + ' already exists');
            return false;
        }
    }

    if ('files' in fptr) {
        for (let i = fptr.files.length - 1; i >= 0; i--) {
            let file = fptr.files[i];
            if (file.name.split('.').pop() != 'ged') {
                updateStatus('File must be a GEDCOM file');
                return false;
            }

        }
        updateStatus("Successfuly uploaded " + fptr.files[0].name);
        return true;
    }
    updateStatus('Invalid File');
    return false;
}
/************** Done Validating File **************/

//ADD INDIVIDUAL
 function addInd () {

        let fileName = document.getElementById('ADDFile').innerHTML;

        if (fileName == 'No File Selected') {
            updateStatus('Please select a file to add to');
            return;
        }

        let tokens = fileName.split(" ");
        if (tokens.length == 3) {
            fileName = tokens[2];
        } else {
            return;
        }

        let firstName =  document.getElementsByName("firstName")[0].value;
        let lastName =  document.getElementsByName("lastName")[0].value;

        if (firstName.indexOf('"') != -1) {
            updateStatus(firstName +" <- Can't contain quotations in a name");
            return;
        } else if (lastName.indexOf('"') != -1) {
            updateStatus(lastName +" <- Can't contain quotations in a last name");
            return;
        }

        console.log(AllIndividuals);
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/addIndividual?fileName=' + fileName + '&firstName=' + firstName + '&lastName=' + lastName,   //The server endpoint we are connecting to
            success: function (data) {
                    PVFileName(fileName);
                    document.getElementById('addingInd').reset();
                    document.getElementById('ADDFile').innerHTML = 'No File Selected';
                    onLoad();
                    updateStatus(firstName +" " + lastName + " has been added to " + fileName);

                    let individual = new Object();
                    individual.fileName = fileName;
                    individual.surname = lastName;
                    individual.givenName = firstName;
                    individual.sex = "-";
                    individual.famSize = 1;
                    AllIndividuals.push(individual);
                    console.log(AllIndividuals);
            },
            fail: function(error) {
                updateStatus(firstName +" " + lastName + " was NOT added to " + fileName);
                console.log(error);
            }
        });

 }

 //getDescendants
 function getDescendants() {

    $('.decDisplay').html('');

    let fileName = document.getElementById('DECFile').innerHTML;

    if (fileName == 'No File Selected') {
        updateStatus("No file selected.");
        return;
    }

    let tokens = fileName.split(" ");
    if (tokens.length == 3) {
        fileName = tokens[2];
    } else {
        return;
    }


    let firstName =  document.getElementsByName("firstName")[1].value;
    let lastName =  document.getElementsByName("lastName")[1].value;
    let maxGen =  document.getElementsByName("maxGen")[0].value;

    if (isNaN(parseFloat(maxGen)) && !isFinite(maxGen)) {
        updateStatus("Max Generation must be an integer.");
        return;
    }

    if (maxGen < 0) {
        updateStatus("Max Generation must 0 or greater.");
        return;
    }

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getDescendants?fileName=' + fileName + '&firstName=' + firstName + '&lastName=' + lastName + '&maxGen=' + maxGen,   //The server endpoint we are connecting to
        success: function (data) {
            if (data.length > 0) {
                if (data[0] == "DNE") {
                    updateStatus(firstName+" "+lastName+ " does not exist in "+fileName);
                } else if (data[0] != "ERROR") {
                    for (var i = 0; i < data.length; i++) {
                        updateDecTable(data[i],i+1);
                    } 
                }
            } else {
                $('.decDisplay').html(firstName+" "+lastName+ " has no descendants.");
                updateStatus(firstName+" "+lastName+ " has no descendants.");
            }
            // console.log(data);
        },
        fail: function(error) {
            $('.decDisplay').html("No Descendants.");
            console.log(error);
        }
    });

 }

function updateDecTable(obj, genNum) {

    $('.decDisplay').append('<table><tr><th>Generation ' + genNum + '</th></tr></table>');
    for (var i = 0; i < obj.length; i++) {
        $('.decDisplay').append('<tr><th>' + obj[i].givenName + '</th><th> ' + obj[i].surname + '</th><tr>');
    }
}


//getAncestors
function getAncestors() {

    $('.ancDisplay').html('');

    let fileName = document.getElementById('ANCFile').innerHTML;

    if (fileName == 'No File Selected') {
        updateStatus("No file selected.");
        return;
    }

    let tokens = fileName.split(" ");
    if (tokens.length == 3) {
        fileName = tokens[2];
    } else {
        updateStatus("Sorry, something went wrong ...");
        return;
    }


    let firstName =  document.getElementsByName("firstName")[2].value;
    let lastName =  document.getElementsByName("lastName")[2].value;
    let maxGen =  document.getElementsByName("maxGen")[1].value;

    if (isNaN(parseFloat(maxGen)) && !isFinite(maxGen)) {
        updateStatus("Max Generation must be an integer.");
        return;
    }

    if (maxGen < 0) {
        updateStatus("Max Generation must 0 or greater.");
        return;
    }

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getAncestors?fileName=' + fileName + '&firstName=' + firstName + '&lastName=' + lastName + '&maxGen=' + maxGen,   //The server endpoint we are connecting to
        success: function (data) {
            if (data.length > 0) {
                if (data[0] == "DNE") {
                    updateStatus(firstName+" "+lastName+ " does not exist in "+fileName);
                } else if (data[0] != "ERROR") {
                    for (var i = 0; i < data.length; i++) {
                        //console.log(data[i]);
                        updateAncTable(data[i],i+1);
                    }
                }
            } else {
                $('.ancDisplay').html(firstName+" "+lastName+ " has no ancestors.");
                updateStatus(firstName+" "+lastName+ " has no ancestors.");
            }
            // console.log(data);
        },
        fail: function(error) {
            $('.ancDisplay').html("No Ancestors");
            console.log(error);
        }
    });

}


function updateAncTable(obj, genNum) {

    $('.ancDisplay').append('<table><tr><th>Generation ' + genNum + '</th></tr></table>');
    for (var i = 0; i < obj.length; i++) {
        $('.ancDisplay').append('<tr><th>' + obj[i].givenName + '</th><th> ' + obj[i].surname + '</th><tr>');
    }
}

function createGEDCOM() {

    let validForm = true;
    let fileName =  document.getElementsByName("fileName")[0].value;
    if (fileName.length < 5) {
        updateStatus('Invalid File Name');
        return;
    } else if (fileName.indexOf('.ged') == -1) {
        updateStatus('File Name must include the ".ged" extension');
        return;
    } 
    let space = fileName.split(" ");
    let space2 = fileName.split("\t");
    if (space.length != 1 || space2.length != 1) {
        updateStatus('File Name must not include spaces');
        return;
    }
    let ext = fileName.split(".");
    if (ext.length != 2) {
        updateStatus('Invalid File format');
        return;
    }


    let numbers = "0123456789";
    for (var i = 0; i < numbers.length; i++) {
       if(numbers[i] == fileName[0]) {
        updateStatus("File Name can't start with a number");
        return;
       }
    }


    let source =  document.getElementsByName("source")[0].value;
    if (source.length == 0) {
        updateStatus("Source is a required field");
        return;
    } else if (source.length > 249) {
        updateStatus("Source is too long (249 characters max)");
        return;
    }

    let encoding =  document.getElementsByName("enc")[0].value;
    encoding = encoding.toUpperCase();
    if (['ANSEL', 'UTF8', 'UTF-8', 'UNICODE', 'ASCII'].indexOf(encoding) == -1) {
        updateStatus("Invalid Encoding");
        return;
    }


    let subName =  document.getElementsByName("subName")[0].value;
    if (subName.length == 0) {
        updateStatus("Submitter Name is a required field");
        return;
    } else if (subName.length > 61) {
        updateStatus("Submitter Name is too long (61 characters max)");
        return;
    }

    let subAddress =  document.getElementsByName("subAddress")[0].value;
    if (subAddress.length > 500) {
        updateStatus("Submitter Address too long (500 characters max)");
        return;
    }


    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/create?fileName=' + fileName + '&source=' + source + '&enc=' + encoding + '&subName=' + subName + '&subAddress=' + subAddress,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {

            if (data[0] != 'ERROR') {
                updateStatus(fileName + " has successfully uploaded");
                document.getElementById('createGED').reset();
                onLoad();
            } else {
                updateStatus(fileName + " failed to upload");
            }

            //We write the object to the console to show that the request was successful
           //console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            updateStatus(fileName + " failed to upload");
            console.log(error); 
        }
    });

}

/******************* A4 CODE *******************/

function clearData () {

    $.ajax({
        type: 'get',         //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/clearData',   //The server endpoint we are connecting to
        success: function (data) {
            updateStatus("Database has " + 0 + " files and " + 0 + " individuals.");
        },
        fail: function(error) {
            console.log(error);
        }
    });
    
}


function storeFiles (flag) {

    clearData();
    document.getElementById("storeData").disabled = true;
    //onLoad();   
    updateStatus("Loading ...");
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/updateTables?AllFileData=' + JSON.stringify(AllFileData) + '&flag=' + flag,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            createChildTable(1);
            //We write the object to the console to show that the request was successful
           //console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error);
            document.getElementById("storeData").disabled = false;
        }
    });


}

function createChildTable(flag) {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/updateTables?AllFileData=' + JSON.stringify(AllIndividuals) + '&flag=' + flag,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            updateStatus("Database has " + AllFileData.length + " files and " + AllIndividuals.length + " individuals.");
            document.getElementById("storeData").disabled = false;
            //We write the object to the console to show that the request was successful
           //console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            updateStatus("Could not store data");
            console.log(error); 
            document.getElementById("storeData").disabled = false;
        }
    });

}


function getDBS() {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getDBS',   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            if (data.length > 1) {
                updateStatus("Database has " + data[0] + " files and " + data[1] + " individuals.");
            }
            //We write the object to the console to show that the request was successful
           //console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });
}

/** queries **/

function fileSelect() {


    let modal = document.getElementById('fileMenu');
    let span = document.getElementsByClassName("exit")[0];
    modal.style.display = "block";

    span.onclick = function() {
        $('.filePicker').html('<span class="exit">&times;</span>');
        modal.style.display = "none";
    }

    window.onclick = function(event) {
        if (event.target == modal) {
            $('.filePicker').html('<span class="exit">&times;</span>');
            modal.style.display = "none";
            
        }
    }

    getFilesFromDB();
    
}

function getFilesFromDB() {
    filesInDB = [];
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFilesFromDB?',   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            for (var i = 0; i < data.length; i++) {
                filesInDB.push(data[i].file_name);
            }
            fillFileSelect();
        },
        fail: function(error) {
            console.log(error); 
        }
    });

}

function fillFileSelect () {

    let selections = document.getElementsByClassName("filePicker")[0].innerHTML;
    selections += '<h2><u>Select a File</u></h2>';
    for (var i = 0; i < filesInDB.length; i++) {
        selections += '<button class="btn btn2 btn-secondary" type="button" onclick="selectFile(\''+ filesInDB[i] +'\')"> '+ filesInDB[i] +' </button>';
    }

    $('.filePicker').html(selections);

    let modal = document.getElementById('fileMenu');
    let span = document.getElementsByClassName("exit")[0];
    span.onclick = function() {
        $('.filePicker').html('<span class="exit">&times;</span>');
        modal.style.display = "none";
    }

}

function selectFile (fileName) {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getIndsFromFile?fName=' + fileName,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            $('.tableType').html('Individuals from '+fileName+':');

            $('.resultPanel').html('');
            $('.resultPanel').append('<table class="table table-striped table-bordered table-hover table-condensed" id="LNTable">');
            if (data.length == 0)
                document.getElementById('LNTable').innerHTML = 'No Individuals';
            else {
                document.getElementById('LNTable').innerHTML = '<tr><th><u>Firstname</u></th><th><u>Lastname</u></th><th><u>Sex</u></th><th><u>Family Size</u></th></tr>';
                let table = document.getElementById('LNTable').innerHTML;
                for (var i = 0; i < data.length; i++) {
                    table += '<tr><th>'+data[i].given_name+'</th><th>'+ data[i].surname +'</th><th>'+data[i].sex+'</th><th>'+data[i].fam_size+'</th></tr>';
                }
                document.getElementById('LNTable').innerHTML = table;
            }
            let modal = document.getElementById('fileMenu');
            $('.filePicker').html('<span class="exit">&times;</span>');
            modal.style.display = "none";

        },
        fail: function(error) {
            console.log(error); 
        }
    });

}

//Personal Query
function personalQuery (flag) {

    let command = "SELECT ";
    if (flag == 0) 
        command += document.getElementById("personalSelect").value;
    else if (flag == 1) command = 'SELECT * FROM INDIVIDUAL ORDER BY surname';
    else if (flag == 2) command = 'SELECT * FROM INDIVIDUAL ORDER BY given_name';
    else if (flag == 3) command = 'SELECT * FROM INDIVIDUAL ORDER BY fam_size';
    else if (flag == 4) command = 'SELECT * FROM INDIVIDUAL ORDER BY sex DESC';
    else if (flag == 5) command = 'SELECT * FROM FILE';
    else if (flag == 6) command = 'DESCRIBE FILE';
    else if (flag == 7) command = 'DESCRIBE INDIVIDUAL';


    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/personalQuery?command=' + command,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {
            $('.tableType').html('');
            if (data.length == 0) {
                updateStatus('No data returned');
                return;
            }

            if (data[0] == "ERROR") {
                updateStatus(data[1].code);
                return;
            } else {
                updateStatus("Success");
            }

            let keys = Object.keys(data[0]);
            //create table
            let table = "<tr>";
            $('.resultPanel').html('');
            $('.resultPanel').append('<table class="table table-striped table-bordered table-hover table-condensed" id="PTable">');
            for (var i = 0; i < keys.length; i++) {
                table += '<th><u>'+ keys[i] +'</u></th>'
            }
            table += '</tr>';

            for (let i = 0; i < data.length; i++) {
                table += '<tr>';
                for (let j = 0; j < keys.length; j++) {
                    table += '<th>'+data[i][Object.keys(data[i])[j]]+'</th>';

                }
                table += '</tr>';
            }
            document.getElementById('PTable').innerHTML = table;


        },
        fail: function(error) {
            console.log(error); 
        }
    });

}

var helpFlag = 0;
var helpFlag2 = 0;
function displayHelp () {

    let modal = document.getElementById('helpMenu');
    let span = document.getElementsByClassName("exit")[1];
    modal.style.display = "block";

    span.onclick = function() {
            $('describeFile').html('<h2><u>DESCRIBE FILE</u></h2><table class="table table-striped table-bordered table-hover table-condensed" id="helpDF"></table>');
            $('describeIndividual').html('<h2><u>DESCRIBE INDIVIDUAL</u></h2><table class="table table-striped table-bordered table-hover table-condensed" id="helpDI"></table>');
        modal.style.display = "none";
    }

    window.onclick = function(event) {
        if (event.target == modal) {
            $('describeFile').html('<h2><u>DESCRIBE FILE</u></h2><table class="table table-striped table-bordered table-hover table-condensed" id="helpDF"></table>');
            $('describeIndividual').html('<h2><u>DESCRIBE INDIVIDUAL</u></h2><table class="table table-striped table-bordered table-hover table-condensed" id="helpDI"></table>');
            modal.style.display = "none";
            
        }
    }

    helpFlag = 1;
    loadHelp(1);
    helpFlag2 = 1;
    loadHelp(2);


}

function loadHelp (flag) {

    if (flag == 1) command = 'DESCRIBE FILE';
    else if (flag == 2) command = 'DESCRIBE INDIVIDUAL';

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/personalQuery?command=' + command,   //The server endpoint we are connecting to   //The server endpoint we are connecting to
        success: function (data) {

            if (data.length == 0) {
                updateStatus('No data returned');
                return;
            }

            if (data[0] == "ERROR") {
                updateStatus(data[1].code);
                return;
            } else {
                updateStatus("Success");
            }

            let keys = Object.keys(data[0]);
            //create table
            let table = "<tr>";
            for (var i = 0; i < keys.length; i++) {
                table += '<th><u>'+ keys[i] +'</u></th>'
            }
            table += '</tr>';

            for (let i = 0; i < data.length; i++) {
                table += '<tr>';
                for (let j = 0; j < keys.length; j++) {
                    table += '<th>'+data[i][Object.keys(data[i])[j]]+'</th>';

                }
                table += '</tr>';
            }
            if (helpFlag == 1) {
                document.getElementById('helpDF').innerHTML = table;
                helpFlag = 0;
            } else if (helpFlag2 == 1) {
                document.getElementById('helpDI').innerHTML = table;
                helpFlag2 = 0;
            }


        },
        fail: function(error) {
            console.log(error); 
        }
    });

}




