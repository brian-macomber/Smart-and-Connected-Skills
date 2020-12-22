//modules
var level = require("level");
const lineReader = require("line-reader");

// // Create or open the underlying LevelDB store
var db = level("./skill26_mydb", { valueEncoding: "json" });

// Read from smoke.txt and put each line into the database
//variable that holds data
var count = 0;
lineReader.eachLine("smoke.txt", function (line) {
  //don't want the first line of the file
  if (count != 0) {
    linestr = String(line).split("\t");

    var value = [
      {
        Time: parseInt(linestr[0]),
        ID: parseInt(linestr[1]),
        Smoke: parseInt(linestr[2]),
        Temp: parseFloat(linestr[3]),
      },
    ];

    // Put structure into database based on key == count, and value
    db.put([count], value, function (err) {
      if (err) return console.log("Ooops!", err); // some kind of I/O error
    });
  }

  count++;
});

var delayInMilliseconds = 2000;
setTimeout(function () {}, delayInMilliseconds);

// query from database - sensor ids, smoke, temperature
console.log(
  db.get(1, function (err, value) {
    //change # for desired query
    if (err) {
      console.error("null");
    } else {
      console.log("Query 1: ", value);
    }
  })
);
