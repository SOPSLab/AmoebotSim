Scripting
=========

This scripting reference is for researchers 🧪 and developers 💻 learning how to write custom JavaScript experiments for AmoebotSim.

.. todo::
  Add an introduction to scripting in AmoebotSim.


.. _script-api:

Scripting API
-------------

The following is a list of all recognized commands.

.. js:function:: log(msg, error)

  :param string msg: A message to log to AmoebotSim's interface.
  :param boolean error: ``true`` if and only if this is an error message.

  Emits the message ``msg`` to the console, and can be denoted as an error message by setting ``error`` to ``true``.

.. todo::
  Add all recognized commands from ``ScriptEngine`` and ``ScriptInterface``.
