'use strict';

const baseDomain = 'http://blynk-cloud.com/[YOUR KEY HERE]'

// const Alexa = require('alexa-sdk');
const Alexa = require('ask-sdk-core');
var http = require('http');


function makeRequest(path){
    return new Promise(((resolve, reject) => {
    http.get(`${baseDomain}${path}`, (resp) => {
    let data = '';

    // A chunk of data has been recieved.
    resp.on('data', (chunk) => {
        data += chunk;
    });

    // The whole response has been received. Print out the result.
    resp.on('end', () => {
        resolve();
    });

    }).on("error", (err) => {
        
        reject(err);
    });
}));

}

const valuesMap = {
    "confetti" : 1,
    "arcade" :4,
    "juggle" : 3,
    "jungle" : 3,
    "dots" : 2,
    "off" : 0,
    "loud" : 5,
    "crazy" : 5,
    "random" : 6
}

function getRandomInt(max) {
  return Math.floor(Math.random() * Math.floor(max));
}  

const NeoIntentHandler = {
    canHandle(handlerInput) {
        return handlerInput.requestEnvelope.request.type === 'IntentRequest'
            && handlerInput.requestEnvelope.request.intent.name === 'NeoIntent';
    },
    async handle(handlerInput) {  
            let lightName = handlerInput.requestEnvelope.request.intent.slots.animation.value;
            let lightVal = (valuesMap[lightName] === 6) ? (getRandomInt(3)+1) : valuesMap[lightName];
            
            await makeRequest(`/update/v0?value=${lightVal}`);
                const speakOutput = `Let's get this ${lightName} light show started.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .getResponse();
            
    }
};

const LaunchRequestHandler = {
    canHandle(handlerInput) {
        return handlerInput.requestEnvelope.request.type === 'LaunchRequest';
    },
    handle(handlerInput) {
        const speakOutput = 'Welcome, ask me how to work your light frame.';
        return handlerInput.responseBuilder
            .speak(speakOutput)
            .getResponse();

           

    }
};

const HelpIntentHandler = {
    canHandle(handlerInput) {
        return handlerInput.requestEnvelope.request.type === 'IntentRequest'
            && handlerInput.requestEnvelope.request.intent.name === 'AMAZON.HelpIntent';
    },
    handle(handlerInput) {
        let names = "";
        for(let key in valuesMap){
            names += key +",";
        }
        const speakOutput = `Welcome, ask me to run a light show with names like, ${names}`;

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};
const CancelAndStopIntentHandler = {
    canHandle(handlerInput) {
        return handlerInput.requestEnvelope.request.type === 'IntentRequest'
            && (handlerInput.requestEnvelope.request.intent.name === 'AMAZON.CancelIntent'
                || handlerInput.requestEnvelope.request.intent.name === 'AMAZON.StopIntent');
    },
    async handle(handlerInput) {  
    
            await makeRequest(`/update/v0?value=0`);
                const speakOutput = `Bummer. Stopping the light frame show.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .getResponse();
            
    }
};
const SessionEndedRequestHandler = {
    canHandle(handlerInput) {
        return handlerInput.requestEnvelope.request.type === 'SessionEndedRequest';
    },
    handle(handlerInput) {
        // Any cleanup logic goes here.
        return handlerInput.responseBuilder.getResponse();
    }
};

// Generic error handling to capture any syntax or routing errors. If you receive an error
// stating the request handler chain is not found, you have not implemented a handler for
// the intent being invoked or included it in the skill builder below.
const ErrorHandler = {
    canHandle() {
        return true;
    },
    handle(handlerInput, error) {
        console.log(`~~~~ Error handled: ${error.message}`);
        const speakOutput = `Sorry, I couldn't understand what you said. Please try again.`;

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

// The SkillBuilder acts as the entry point for your skill, routing all request and response
// payloads to the handlers above. Make sure any new handlers or interceptors you've
// defined are included below. The order matters - they're processed top to bottom.
exports.handler = Alexa.SkillBuilders.custom()
    .addRequestHandlers(
        LaunchRequestHandler,
        NeoIntentHandler,
        HelpIntentHandler,
        CancelAndStopIntentHandler,
        SessionEndedRequestHandler,
        // IntentReflectorHandler
        ) // make sure IntentReflectorHandler is last so it doesn't override your custom intent handlers
    .addErrorHandlers(
        ErrorHandler)
    .lambda();