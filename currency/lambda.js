const xng = require('./service.js');
const aws = require('aws-sdk'); //remove when uploading into AWS Lambda

const dateFormat = require('dateformat');
const now = new Date();



exports.serviceHandler = function(event, context, callback) {
  const to = event['queryStringParameters']['to'] || 'USD';
  const from   = event['queryStringParameters']['from'];
  const date = event['queryStringParameters']['date'] || dateFormat(now, "isoDate");
  const amount = event['queryStringParameters']['amount'] || 1;
  xng.fetchCurrency(to, from, date, amount).then(function(data) {
    console.log(data);
    callback(null, {
      statusCode: 200,
      headers: {
        'Content-Type': 'application/json'
      },
      body: data
    });
  }).catch(function(error) {
    console.log(error);
    callback(null, {
      statusCode: 500,
      headers: {
        'Content-Type': 'application/json'
      },
      body: error
    });
  });
};
