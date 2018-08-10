const aws = require('aws-sdk'); //remove when uploading into AWS Lambda

const dateFormat = require('dateformat');
const now = new Date();

const key   = 'INSERT_KEY';
const url   = 'https://openexchangerates.org';
const table = 'INSERT_TABLE_NAME';

aws.config.update({
  region:   "INSERT_REGION",
  accessKeyId: 'INSERT_KEY',
  secretAccessKey: 'INSERT_SECRET'
});

var db = new aws.DynamoDB();

module.exports = {
  fetchCurrency: function(to, from, date, amount) {
  if (to.length != 3 || from.length != 3 || to == from){
    console.log("Error, incorrect parameters submitted.");
    return new Promise(function(resolve, reject) {
      "Error, incorrect parameters submitted.";
    });
  } else {
  if (to == 'USD') {
    const params = {
      TableName: table,
      Key: {
        'mykey': {S: date + '_' + to + '_' + from}
      }
    };
    console.log(params);
    return new Promise(function(resolve, reject) {
      db.getItem(params, function(err, data) {
        console.log(params);
        if (err) {
          console.log("Error", err);
          reject(err);
        } else {
          console.log("Get " + params.Key.mykey, data);
          var convertedRate = amount;
          convertedRate = (data.Item.inverse.N * convertedRate);
          console.log("Converted Rate: " + convertedRate);
          console.log("Success", JSON.stringify(data));
          console.log("Success", data);
          console.log(JSON.stringify({to: to, from: from, amount: data.Item.inverse.N, inverse: data.Item.amount.N, converted_rate: convertedRate.toString()}));
          data = JSON.stringify({to: to, from: from, amount: data.Item.inverse.N, inverse: data.Item.amount.N, converted_rate: convertedRate.toString()});
          resolve(data);
        }
      });
    });
  } else {
    const params = {
      TableName: table,
      Key: {
        'mykey': {S: date + '_' + from + '_' + to}
      }
    };
    return new Promise(function(resolve, reject) {
      db.getItem(params, function(err, data) {
        console.log(params);
        if (err) {
          console.log("Error", err);
          reject(err);
        } else {
          console.log("Get " + params.Key.mykey, data);
          var convertedRate = amount;
          convertedRate = (data.Item.amount.N * convertedRate);
          console.log("Converted Rate: " + convertedRate);
          console.log("Success", JSON.stringify(data));
          console.log("Success", data);
          console.log(JSON.stringify({to: to, from: from, amount: data.Item.amount.N, inverse: data.Item.inverse.N, converted_rate: convertedRate.toString()}));
          data = JSON.stringify({to: to, from: from, amount: data.Item.amount.N, inverse: data.Item.inverse.N, converted_rate: convertedRate.toString()});
          resolve(data);
        }
      });
    });
   }
  }
 }
}


