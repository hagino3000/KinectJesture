/**
 * WebSocket wrapper for multi use
 *
 * WebSocket sends next format data (JSON)
 * <code>
 * {
 *   "eventName" : "imageUrl",
 *   "data" : {
 *     "lastRetreiveTime" : "1171815102"
 *   },
 *   "pid" : 0,
 *   "socketKey" : "8GzBJq42m9"
 * }
 * </code>
 * pid is sequencial number in this socket.
 * socketKey is a random string using to identify this socket.
 *
 * And receive data format is
 * <code>
 * {
 *   "eventName" : "imageUrl",
 *   "data" : {[
 *      ....
 *   ]},
 *   "pid" : 0
 * }
 * </code>
 *
 *
 * Usage:
 * <code>
 * var ws = new WSMessage({
 *   url:"ws://localhost:8080",
 *   listeners : {
 *     open : function(){console.info('onopen!!')},
 *     close : function(){console.info('onclose!!')}
 *   }
 * });
 * // wait push event from server
 * ws.on('newUser', function(data) {
 *   console.dir(data);
 * });
 *
 * // data request and reveive
 * ws.on('imageUrl', handleImageUrl); 
 * ws.send('imageUrl', {lastRetrieveTime : 0000000000});
 *
 * function handleImageUrl(data) {
 *  doSomething(data);
 * }
 *
 * // unfollow event
 * ws.un('imageUrl', handleImageUrl);
 *
 * // close
 * ws.close();
 * </code>
 *
 * @see https://github.com/hagino3000/websocket_messaging
 */
WSMessage = function(config) {
  WSMessage.prototype.initialize.call(this, config);
}

WSMessage.prototype = {
  events : null,
  pid : 0,
  socketKey : null,
  useSocketIoJS : false,

  /**
   * @constructor
   * @param object config
   * <code>
   * {
   *   url : "ws://chaos.yuiseki.net:4569",
   *   autoRecovery : true
   * }
   * </code>
   */
  initialize : function(config) {
    this.alive = false;
    this.events = {};
    this.autoRecovery = config.autoRecovery || false;
    this.config = config;
    if (config.listeners) {
      var listeners = config.listeners;
      for (action in listeners) {
        if (typeof(listeners[action]) == 'function' || listeners[action] instanceof Function){
          this.on(action, listeners[action]);
        } else {
          this.on(action, listeners[action].fn, listeners[action].scope);
        }
      }
    }
    this.socketKey = this._createKey();
    this.useSocketIoJS = window.io;
    this._open();
  },

  _createKey : function() {
    var result = '';
    var source = 'abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    for (var i=0; i<10; i++) {
      result+=source[Math.floor(Math.random()*source.length)];
    }
    return result;
  },

  /**
   * @private
   */
  _open : function() {
    var ws;
    console.info('start open:' + this.config.url);
    try {
      if (this.useSocketIoJS) {
        var m =  this.config.url.match(/ws:\/\/(.*):([0-9]*)/);
        var host =  m[1];
        var port = m[2];
        ws = new io.Socket(host, {
          port : port
        });
        ws.on('connect', this._bind(this._onopen, this));
        ws.on('message', this._bind(this._onmessage, this));
        ws.on('disconnect', this._bind(this._onclose, this));
        ws.connect();
      } else {
        ws = new WebSocket(this.config.url);
        ws.onopen = this._bind(this._onopen, this);
        ws.onmessage = this._bind(this._onmessage, this);
        ws.onclose = this._bind(this._onclose, this);
      }
      this.ws = ws;
    } catch (e) {
      console.error('Failed to open the websocket connection');
      console.error(e);
    }
  },

  /**
   * @private
   */
  _onopen : function() {
    this.alive = true;
    this._fire('open');
  },

  count : 0,
  lastT : new Date(),

  /**
   * @private
   */
  _onmessage : function(data) {
    if (!this.useSocketIoJS) {
      data = data.data;
    }
    this.count++;
    var cTime = new Date();
    if ((cTime - this.lastT) > 1000) {
      console.info(this.count);
      this.count = 0;
      this.lastT = cTime;
    }
    try {
      var d = JSON.parse(data);
      var eventName = d.type.toLowerCase();
      this._fire(eventName, d, d.socketKey, d.pid);
    } catch(e) {
      console.error(e);
    }
  },

  /**
   * @private
   */
  _fire : function(eventName, data, socketKey, pid) {
    data = data || {};
    var fns = this.events[eventName];
    if (fns) {
      for (var i=0, len=fns.length; i<len; i++) {
        fns[i].fn.call(fns[i].scope, data.data, socketKey, pid);
      }
    }
  },

  /**
   * @private
   */
  _onclose : function() {
    this.alive = false;
    this._fire('close');
    if (this.autoRecovery) {
      // retry after 10 seconds
      setTimeout(this._bind(this._open, this), 10000);
    }
  },
  
  /**
   * Send message to server
   * @public
   */
  send : function(eventName, data) {
    data = data || {};
    this.ws.send(JSON.stringify({
      eventName : eventName,
      data : data,
      pid : this.pid++,
      socketKey : this.socketKey
    }));
  },

  /**
   * Close this socket
   * @public
   */
  close : function() {
    this.autoRecovery = false;
    this.ws.close();
  },

  /**
   * Add event listener
   * @public
   */
  on : function(eventName, fn, scope) {
    eventName = eventName.toLowerCase();
    if (!this.events[eventName]) {
      this.events[eventName] = [];
    }

    var fns = this.events[eventName];
    fns.push({
      fn : fn, scope : scope
    });
  },

  /**
   * Remove event listener
   * @public
   */
  un : function(eventName, fn, scope) {
    eventName = eventName.toLowerCase();
    var fns = this.events[eventName];
    if (fns) {
      for (var i=0; i<fns.length; i++) {
        if (fns[i].fn == fn && fns[i].scope == scope) {
          fns.splice(i, 1);
          return true;
        }
      }
    }
    return false;
  },

  /**
   * Remove all event listeners
   */
  purgeListeners : function() {
    this.events = {};
  },

  // utilities
  _bind : function(fn, thisObj, args) {
    if (args == undefined) {
      return function(){fn.apply(thisObj, arguments)}
    } else {
      return function(){fn.apply(thisObj, args)}
    }
  }
}
