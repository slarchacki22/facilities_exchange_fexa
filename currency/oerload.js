const axios = require('axios');
const aws = require('aws-sdk'); //remove when uploading into AWS Lambda

const dateFormat = require('dateformat');
const now = new Date();

const key   = 'INSERT_KEY';
const url   = 'https://openexchangerates.org';
const table = 'INSERT_TABLE_NAME';

aws.config.update({
  region:   'INSERT_REGION',
  accessKeyId: 'INSERT_KEY',
  secretAccessKey: 'INSERT_SECRET'
});

var db = new aws.DynamoDB();

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
        //console.log("Success", JSON.stringify(data));
        console.log("Success", data);
        count = JSON.stringify(data["Count"]);
        resolve(count);
      }
    });
  });
}

exports.loadHandler = function(event, context, callback) {
  const date = dateFormat(now, "isoDate");
  const url  = rateURL(date);

  scanRate(date).then(function(count) {
    console.log("Count is " + count);
    if (count == 0){
      console.log("Count: " + count + " loading currencies for " + date);
      axios.get(url).then(resp => {
      const data = resp.data['rates'];
      for (var key in data) {
        if (key !== 'USD') {
          var inverse = (1/data[key]);
          console.log(date + '_' + key + '_amount_' + data[key] + '_inverse_' + inverse);
          writeRate(date, key, 'USD', data[key], inverse);
        }
      }
      callback(null, {
        statusCode: 200,
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({foo: resp.data['rates']['CAD']})
      });
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
  })
  .catch(function(err) {
    console.log('Error in db.scan ' + err);
  });
}

