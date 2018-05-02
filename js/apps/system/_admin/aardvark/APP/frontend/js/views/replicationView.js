/* jshint browser: true */
/* jshint unused: false */
/* global _, Backbone, btoa, templateEngine, $, window, arangoHelper */
(function () {
  'use strict';

  window.ReplicationView = Backbone.View.extend({
    el: '#content',

    // Modes:
    //  0: No active replication found.
    //  1: Replication per Database found.
    //  2: Replication per Server found.
    //  3: Active-Failover replication found.
    mode: null,

    // general info object, valid in all modes
    info: {
      state: null,
      mode: null,
      level: null,
      role: null,
      msg: 'There are no known issues'
    },

    // nodes info object, valid in active failover mode (3)
    nodes: {
      leader: null,
      followers: []
    },

    template: templateEngine.createTemplate('replicationView.ejs'),

    events: {
      'click #nodes-followers-id span': 'goToApplier',
      'click #repl-follower-table tr': 'goToApplierFromTable'
    },

    render: function () {
      if (this.mode || this.mode === 0) {
        // mode found
        this.$el.html(this.template.render({
          mode: this.mode,
          info: this.info,
          nodes: this.nodes
        }));

        // fetching mode 3 related information
        if (this.mode === 3) {
          this.getActiveFailoverEndpoints();
          this.getLoggerState();
        } else if (this.mode === 2) {
          if (this.info.role === 'leader') {
            this.getLoggerState();
          } else {
            this.getApplierState();
          }
        } else if (this.mode === 1) {
          if (this.info.role === 'leader') {
            this.getLoggerState();
          } else {
            this.getApplierState();
          }
        }
      } else {
        this.getMode(this.render.bind(this));
      }
    },

    goToApplier: function (e) {
      // always system (global applier)
      var endpoint = btoa($(e.currentTarget).attr('data'));
      window.App.navigate('#replication/applier/' + endpoint + btoa('/_system'), {trigger: true});
    },

    goToApplierFromTable: function (e) {
      // per db (single applier)
      var endpoint = btoa(window.location.origin);
      var db = btoa($(e.currentTarget).find('#applier-database-id').html());
      var running = $(e.currentTarget).find('#applier-running-id').html();
      if (running === 'true' || running === true) {
        window.App.navigate('#replication/applier/' + endpoint + '/' + db, {trigger: true});
      } else {
        arangoHelper.arangoMessage('Replication', 'This applier is not running.');
      }
    },

    getActiveFailoverEndpoints: function () {
      var self = this;
      $.ajax({
        type: 'GET',
        cache: false,
        url: arangoHelper.databaseUrl('/_api/cluster/endpoints'),
        contentType: 'application/json',
        success: function (data) {
          if (data.endpoints) {
            self.renderEndpoints(data.endpoints);
          } else {
            self.renderEndpoints();
          }
        },
        error: function () {
          self.renderEndpoints();
        }
      });
    },

    renderEndpoints: function (endpoints) {
      var self = this;

      if (endpoints) {
        var leader = endpoints[0];
        var followers = endpoints.slice(1, endpoints.length);

        $('#nodes-leader-id').html(leader.endpoint);
        $('#nodes-followers-id').html('');
        _.each(followers, function (follower) {
          $('#nodes-followers-id').append('<span data="' + self.parseEndpoint(follower.endpoint, true) + '">' + follower.endpoint + '</span>');
        });
      } else {
        $('#nodes-leader-id').html('Error');
        $('#nodes-followers-id').html('Error');
      }
    },

    parseEndpoint: function (endpoint, url) {
      var parsedEndpoint;
      if (endpoint.slice(6, 11) === '[::1]') {
        parsedEndpoint = window.location.host.split(':')[0] + ':' + endpoint.split(':')[4];
      } else if (endpoint.slice(0, 6) === 'tcp://') {
        parsedEndpoint = 'http://' + endpoint.slice(6, endpoint.length);
      } else if (endpoint.slice(0, 6) === 'ssl://') {
        parsedEndpoint = 'https://' + endpoint.slice(6, endpoint.length);
      }

      if (url) {
        parsedEndpoint = window.location.protocol + '//' + parsedEndpoint;
      }

      if (!parsedEndpoint) {
        return endpoint;
      }

      return parsedEndpoint;
    },

    getLoggerState: function () {
      var self = this;
      $.ajax({
        type: 'GET',
        cache: false,
        url: arangoHelper.databaseUrl('/_api/replication/logger-state'),
        contentType: 'application/json',
        success: function (data) {
          self.renderLoggerState(data.server, data.clients, data.state);
        },
        error: function () {
          arangoHelper.arangoError('Replication', 'Could not fetch the leaders logger state.');
        }
      });
    },

    getApplierState: function () {
      var self = this;
      $.ajax({
        type: 'GET',
        cache: false,
        url: arangoHelper.databaseUrl('/_api/replication/applier-state-all'),
        contentType: 'application/json',
        success: function (data) {
          self.renderApplierState(data);
        },
        error: function () {
          arangoHelper.arangoError('Replication', 'Could not fetch the followers applier state.');
        }
      });
    },

    renderApplierState: function (data) {
      var self = this;
      var endpoint;

      _.each(data, function (applier, db) {
        if (applier.endpoint !== 'undefined' && applier.endpoint) {
          endpoint = self.parseEndpoint(applier.endpoint);
        } else {
          endpoint = 'not available';
        }

        $('#repl-follower-table tbody').append(
        '<tr>' +
          '<td id="applier-database-id">' + db + '</td>' +
          '<td id="applier-running-id">' + applier.state.running + '</td>' +
          '<td>' + applier.state.phase + '</td>' +
          '<td id="applier-endpoint-id">' + endpoint + '</td>' +
          '<td>' + applier.server.version + '</td>' +
        '</tr>'
        );
      });
    },

    renderLoggerState: function (server, clients, state) {
      if (server && clients && state) {
        // render logger information
        $('#logger-running-id').html(state.running);
        $('#logger-version-id').html(server.version);
        $('#logger-serverid-id').html(server.serverId);
        $('#logger-time-id').html(state.time);
        $('#logger-lastLogTick-id').html(state.lastLogTick);
        $('#logger-lastUncommitedLogTick-id').html(state.lastUncommittedLogTick);
        $('#logger-totalEvents-id').html(state.totalEvents);
        // render client information
        $('#repl-logger-clients tbody').html('');
        _.each(clients, function (client) {
          $('#repl-logger-clients tbody').append(
            '<tr><td>' + client.serverId + '</td>' +
            '<td>' + client.time + '</td>' +
            '<td>' + client.lastServedTick + '</td></tr>'
          );
        });
      } else {
        $('#logger-running-id').html('Error');
        $('#logger-endpoint-id').html('Error');
        $('#logger-version-id').html('Error');
        $('#logger-serverid-id').html('Error');
        $('#logger-time-id').html('Error');
      }
    },

    getMode: function (callback) {
      var self = this;
      $.ajax({
        type: 'GET',
        cache: false,
        url: arangoHelper.databaseUrl('/_admin/aardvark/replication/mode'),
        contentType: 'application/json',
        success: function (data) {
          if (data.mode || data.mode === 0) {
            if (Number.isInteger(data.mode)) {
              self.mode = data.mode;
              self.info.state = 'Replication is enabled';
            } else {
              self.mode = 'undefined';
            }
            if (data.role) {
              self.info.role = data.role;
            }
            if (self.mode === 3) {
              self.info.mode = 'Active-Failover';
              self.info.level = 'Server-wide replication';
            } else if (self.mode === 2) {
              self.info.mode = 'Default';
              self.info.level = 'Server-wide replication';
            } else if (self.mode === 1) {
              self.info.mode = 'Default';
              self.info.level = 'Database-level replication';
            }
          }
          if (callback) {
            callback();
          }
        },
        error: function () {
          arangoHelper.arangoError('Replication', 'Could not fetch the replication state.');
        }
      });
    }

  });
}());
