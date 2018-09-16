'use strict'

let connection = null;

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});


//PUSHING MY IMAGES
app.get('/images/greenBack.jpg',function(req,res){
  res.sendFile(path.join(__dirname+'/public/images/greenBack.jpg'));
});
app.get('/images/tree1.png',function(req,res){
  res.sendFile(path.join(__dirname+'/public/images/tree1.png'));
});
app.get('/images/tree2.png',function(req,res){
  res.sendFile(path.join(__dirname+'/public/images/tree2.png'));
});
//Completed pushing style images


// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

/************** GET C FUNCTIONS *************/

let sharedLib = ffi.Library('./sharedLib', {
  'displayIndividuals': [ 'string', ['string'] ],   //return type first, argument list second
  'fileInfo': [ 'string', ['string','string'] ],
  'addIndToFile': [ 'string', ['string','string'] ],
  'returnDecendantsList': [ 'string', ['string','string','int'] ],
  'returnAncestorsList': [ 'string', ['string','string','int'] ],
  'createNewGEDCOM': [ 'string', ['string','string'] ],
});

/************* ACCES TO C FUNCTIONS GRANTED *******/

//Called to return all individuals
app.get('/DisplayData', function(req , res) {
  let fileName  = req.query.fName;
  let dirPath = "uploads/";
  fileName = dirPath+fileName;
  var str = sharedLib.displayIndividuals(fileName);
  res.send(str);
});

app.get('/fileNames', function(req , res) {

  var dirPath = "uploads/";
  var fNames = [];

  fs.readdir(dirPath, (err, files) => {
    files.forEach(file => {
      if(sharedLib.fileInfo(dirPath, file) != "ERROR") {
        fNames.push(file);
      }
    });
    res.send(fNames);
  });

});

app.get('/FileLog', function(req , res) {

  var dirPath = "uploads/";
  var fNames = [];

  fs.readdir(dirPath, (err, files) => {
    files.forEach(file => {
      if (file.indexOf('.ged') != -1) {
        let logInfo = sharedLib.fileInfo(dirPath, file);
       if (logInfo != "ERROR") {
          fNames.push(sharedLib.fileInfo(dirPath, file));
       }
      }
    });
    res.send(fNames);
  });

});


app.get('/addIndividual', function(req , res) {

  let fileName  = req.query.fileName;
  let firstName  = req.query.firstName;
  let lastName  = req.query.lastName;

  let errCode = [];

  let indJson = '{"givenName":"' + firstName + '","surname":"' + lastName + '"}';

  let dirPath = "uploads/";
  fileName = dirPath+fileName;

  var str = sharedLib.addIndToFile(fileName,indJson);
  errCode.push(str);

  res.send(errCode);
});

app.get('/getDescendants', function(req , res) {

  let fileName  = req.query.fileName;
  let firstName  = req.query.firstName;
  let lastName  = req.query.lastName;
  let maxGen  = req.query.maxGen;

  let error = ["ERROR"];
  let error2 = ["DNE"];

  let indJson = '{"givenName":"' + firstName + '","surname":"' + lastName + '"}';

  let dirPath = "uploads/";
  fileName = dirPath+fileName;

  var str = sharedLib.returnDecendantsList(fileName,indJson,maxGen);

  if (str == "ERROR") {
    res.send(error);
  } else if (str == "DNE") {
    res.send(error2);
  } else {
    res.send(str);
  }

});

app.get('/getAncestors', function(req , res) {

  let fileName  = req.query.fileName;
  let firstName  = req.query.firstName;
  let lastName  = req.query.lastName;
  let maxGen  = req.query.maxGen;

  let error = ["ERROR"];
  let error2 = ["DNE"];

  let indJson = '{"givenName":"' + firstName + '","surname":"' + lastName + '"}';

  let dirPath = "uploads/";
  fileName = dirPath+fileName;

  var str = sharedLib.returnAncestorsList(fileName,indJson,maxGen);

  if (str == "ERROR") {
    res.send(error);
  } else if (str == "DNE") {
    res.send(error2);
  } else {
    res.send(str);
  }

});

//fileName + '&source=' + source + '&enc=' + encoding + '&subName=' + subName + '&subAddress=' + subAddress, 
app.get('/create', function(req , res) {

  var dirPath = "uploads/";
  var errCode = [];

  let fileName  = req.query.fileName;
  let source  = req.query.source;
  let encoding = req.query.enc;
  let subName  = req.query.subName;
  let subAddress  = req.query.subAddress;

  let fileJSON = '{"source":"' + source + '","gedcVersion":"5.50","encoding":"' + encoding + '","subName":"' + subName + '","subAddress":"' + subAddress +'"}';
  var str = sharedLib.createNewGEDCOM(dirPath + fileName, fileJSON);
  errCode.push(str);

  res.send(errCode);


});

/********************************** A4 CODE **********************************/

/** TEMP   **/
// const mysql = require('mysql');
// connection = mysql.createConnection({
//     host     : 'dursley.socs.uoguelph.ca',
//     user     : 'rgundu',
//     password : '0988604',
//     database : 'rgundu'
// });

// connection.connect(function(error) {
//   if (error) {
//     throw error;
//   }
// });

/************/

// create table INDIVIDUAL (ind_id int not null auto_increment,  surname varchar(256) not null,  given_name varchar(256), sex varchar(1), fam_size int, source_file int, primary key(ind_id), foreign key(source_file) references FILE(file_id) on delete cascade);
//create table FILE (file_id int not null auto_increment,  file_name varchar(60) not null,  source varchar(250), version varchar(2), encoding varchar(10), sub_name varchar(62),  sub_addr varchar(256), num_individuals int, num_families int, primary key(file_id) );

app.get('/createTables', function(req, res) {

  let create = "create table FILE (file_id int not null auto_increment,  file_name varchar(60) not null,  source varchar(250), version varchar(2), encoding varchar(10), sub_name varchar(62),  sub_addr varchar(256), num_individuals int, num_families int, primary key(file_id) )";
  connection.query(create, function (err, rows, fields) {
      if (err) {
        console.log("Something went wrong. "+err);
        res.send(["ERROR"]);
      } else {
          let create2 = "create table INDIVIDUAL (ind_id int not null auto_increment,  surname varchar(256) not null,  given_name varchar(256), sex varchar(1), fam_size int, source_file int, primary key(ind_id), foreign key(source_file) references FILE(file_id) on delete cascade)";
          connection.query(create2, function (err, rows, fields) {
          if (err) console.log("Something went wrong. "+err);
          res.send([]);
          });
      }
  });

});




app.get('/connectDB', function(req , res) {

  let hst  = req.query.host;
  let usr  = req.query.user;
  let pw = req.query.pw;
  let db  = req.query.db;

  const mysql = require('mysql');

  connection = mysql.createConnection({
      host     : hst,
      user     : usr,
      password : pw,
      database : db
  });
  
  connection.connect(function(error) {
    if (!error) {
      res.send(["Success"]);
    } else res.send(["ERROR"]);
  });

});


//Clear tables
app.get('/clearData', function(req, res) {

  let reset2 = "DELETE FROM FILE";
    connection.query(reset2, function (err, rows, fields) {
          if (err) console.log("Something went wrong. "+err);
  });
  res.send([]);

});


// UPDATE TABLES ON DATABASE //
app.get('/updateTables', function(req , res) {

  let errCode = ["Success1"];

  let fileData  = JSON.parse(req.query.AllFileData);

  let records = [];

  let flag = req.query.flag; //Loading parent or child table

  if (flag == 0 && fileData.length > 0) {

    //console.log('Loading the Parent');

    //Remove table even with references
    let reset = "ALTER TABLE FILE AUTO_INCREMENT = 1";
    connection.query(reset, function (err, rows, fields) {
          if (err) console.log("Something went wrong. "+err);
    });
    ////////

    for (var i = 0; i < fileData.length; i++) {
      let rec = "INSERT INTO FILE VALUES (null, '" + fileData[i].fileName + "', '" + fileData[i].source + "', '" + fileData[i].version + "', '" + fileData[i].encoding + "', '" + fileData[i].subName + "', '" + fileData[i].subAddr + "', " + fileData[i].numInd + ", " + fileData[i].numFam +")"; 
      records.push(rec);
    }

    for (let rec of records) {
      connection.query(rec, function (err, rows, fields) {
          if (err) console.log("Something went wrong. "+err);
      });
    }


  } else if (flag == 1 && fileData.length > 0) {

    errCode = ["Success2"];
    //console.log('Loading the child');

    let reset = "ALTER TABLE INDIVIDUAL AUTO_INCREMENT = 1";
    connection.query(reset, function (err, rows, fields) {
          if (err) console.log("Something went wrong. "+err);
    });

    let k = 0;
    let rec = "";

    for (var i = 0; i < fileData.length; i++) {

        let idSelect = "SELECT file_id FROM FILE WHERE file_name = '" + fileData[i].fileName + "'";
        connection.query(idSelect, function (err, rows, fields) {
          if (err) {
              console.log("Something went wrong. "+err);
          } else {

            if (rows.length == 1) {
              rec = "INSERT INTO INDIVIDUAL VALUES (null, '" + fileData[k].surname + "', '" + fileData[k].givenName + "', '" + fileData[k].sex + "', '" + fileData[k].famSize + "', " + rows[0].file_id + ")"; 
              records.push(rec);
              ++k;

              if (k == fileData.length) {
                for (let rec of records) {
                  connection.query(rec, function (err, rows, fields) {
                      if (err) console.log("Something went wrong. "+err);
                  });
                }
              }
    
            }

          }

        });
    }

  }

  res.send(errCode);

});

//END OF UPDATING DATABASE TABLES //

app.get('/getDBS', function(req , res) {

  let errCode = [];

  let getNumFiles = "SELECT * FROM FILE";
  connection.query(getNumFiles, function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
          errCode.push(rows.length);
        }
  });

  let getNumInds = "SELECT * FROM INDIVIDUAL";
  connection.query(getNumInds, function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
          errCode.push(rows.length);
          res.send(errCode);
        }
        
  });

});

app.get('/getFilesFromDB', function(req , res) {


  let getNumFiles = "SELECT file_name FROM FILE";
  connection.query(getNumFiles, function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        res.send(rows);
  });


});


app.get('/getIndsFromFile', function(req , res) {

  let fileName = req.query.fName;
  let getID = "SELECT * FROM FILE WHERE file_name ='";
  connection.query(getID + fileName +"'", function (err, rows, fields) {

        if (err) {
          console.log("Something went wrong. "+err);
          res.send([]);
        } else if (rows.length > 0){

          let id = rows[0].file_id;
          let getInds = "SELECT * FROM INDIVIDUAL WHERE source_file =";
          connection.query(getInds + id, function (err, rows, fields) {
            if (err) {
              console.log("Something went wrong. "+err);
              res.send(rows);
            } else {
              res.send(rows);
            }     
          });

        } else res.send([]);
        
  });

});


app.get('/personalQuery', function(req , res) {

  let command = req.query.command;
  connection.query(command, function (err, rows, fields) {

    if (err) {
      console.log("Something went wrong. "+err);
      res.send(["ERROR", err]);
    } else res.send(rows);
    
        
  });

});








app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
