const axios = require('axios');
const aws = require('aws-sdk'); //remove when uploading into AWS Lambda

const dateFormat = require('dateformat');
const now = new Date();

const fs = require('fs');
const xml2js = require('xml2js');
const parseString = require('xml2js').parseString;

const key   = 'INSERT_KEY';
const url   = 'https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml';
const table = 'INSERT_TABLE_NAME';

aws.config.update({
  region:   "INSERT_REGION",
  accessKeyId: 'INSERT_KEY',
  secretAccessKey: 'INSERT_SECRET'
});

var db = new aws.DynamoDB();


function writeRate(date, to, from, amount, inverse) {
  const params = {
    TableName: table,
    Item: {
      'mykey':  {S: date + '_' + from + '_' + to},
      'amount': {N: amount.toString()},
      'inverse': {N: inverse.toString()}
    }
  }
  db.putItem(params, function(err, data) {
    if (err) {
      console.log("Error", err);
    } else {
      console.log("Load: " + params.Item.mykey.S, data);
      //console.log("Success", data);
    }
  });
};

function getItem(date, to, from, amount) {
  db.getItem({
    TableName: table,
    Key: {
      'mykey': {S: date + '_' + from + '_' + to}
    }
  }, function(err, data) {
    if (err) {
      console.log("Error", err);
    } else {
      console.log("Success", data);
    }
  });
}

function rateURL(date) {
  return url + '/api/historical/' + date + '.json?app_id=' + key;
}

function scanRate(date) {
  var count = 0;
  const params = {
    TableName: table,
    FilterExpression : "begins_with(mykey, :mykey)",
    ExpressionAttributeValues : {
        ":mykey": {"S": date}  
    }, 
    Select:'COUNT'
  };


  return new Promise(function(resolve, reject) {
    db.scan(params, function(err, data) {
      if (err) {
        console.log("Error", err);
        reject(db.scan);
      } else {
        console.log("Success", data);
        count = JSON.stringify(data["Count"]);
        resolve(count);
      }
    });
  });
}

exports.fetchHandler = function(event, context, callback) {
  const to = event['queryStringParameters']['to'] || 'USD';
  const from   = event['queryStringParameters']['from'];
  const date = event['queryStringParameters']['date'] || dateFormat(now, "isoDate");
  const amount = event['queryStringParameters']['amount'] || 1;
  if ( to == 'USD') {
    const params = {
      TableName: table,
      Key: {
        'mykey': {S: date + '_' + to + '_' + from}
      }
    };
    db.getItem(params, function(err, data) {
      if (err) {
        console.log("Error", err);
        callback(null, {
          statusCode: 500,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({foo: "error"})
        });
      } else if (to.length != 3 || from.length != 3 || to == from){
        console.log("Error, incorrect parameters submitted.");
        callback(null, {
          statusCode: 400,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({foo: "error"})
        }); 
      } else {
        console.log("Get " + params.Key.mykey, data);
        var convertedRate = amount;
        convertedRate = (data.Item.inverse.N * convertedRate);
        console.log("Converted Rate: " + convertedRate);
        callback(null, {
          statusCode: 200,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({to: to, from: from, amount: data.Item.inverse.N, inverse: data.Item.amount.N, converted_rate: convertedRate.toString()})
        });
      }
    });
  } else {
    const params = {
      TableName: table,
      Key: {
        'mykey': {S: date + '_' + from + '_' + to}
      }
    };
    db.getItem(params, function(err, data) {
      if (err) {
        console.log("Error", err);
        callback(null, {
          statusCode: 500,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({foo: "error"})
        });
      } else if (to.length != 3 || from.length != 3 || to == from){
        console.log("Error, incorrect parameters submitted.");
        callback(null, {
          statusCode: 400,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({foo: "error"})
        }); 
      } else {
        console.log("Get " + params.Key.mykey, data);
        var convertedRate = amount;
        convertedRate = (data.Item.amount.N * convertedRate);
        console.log("Converted Rate: " + convertedRate);
        callback(null, {
          statusCode: 200,
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({to: to, from: from, amount: data.Item.amount.N, inverse: data.Item.inverse.N, converted_rate: convertedRate.toString()})
        });
      }
    });
  }
}



exports.loadHandler = function(event, context, callback) {
  const date = dateFormat(now, "isoDate");


  scanRate(date).then(function(count) {
    console.log("Count is " + count);
    if (count != 0){
      console.log("Count: " + count + " loading currencies for " + date);
      axios.get(url).then(resp => {
        console.log(resp);
        parseString(resp.data, {trim: true},  function (error, result) {
          var r = result['gesmes:Envelope']['Cube'][0]['Cube'][0]['Cube']
          for (var i = 0; i < r.length; i++) {
            for (var key in r[i]) {
              if (r[i].hasOwnProperty(key)) {
                if (r[i][key].currency !== 'USD'){
                  var amount =  (1 / (USD/r[i][key].rate));
                  var inverse = (1/amount);
                  console.log(date + '_' + r[i][key].currency + '_amount_' + amount + '_inverse_' + inverse);
                  writeRate(date, r[i][key].currency, 'USD', amount, inverse);
                } else {
                  var amount = (1/USD);
                  console.log(date + '_EUR_amount_' + amount + '_inverse_' + r[i][key].rate);
                  writeRate(date, 'EUR', r[i][key].currency, amount, r[i][key].rate);
                }
              }
            }
          }
          callback(null, {
            statusCode: 200,
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify({foo: resp.data})
          });
        })
      })
    }
  }).catch(error => {
    callback(null, {
      statusCode: 500,
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({foo: error.toString()})
    });
  });
}