
The MonaServer Database System
##############################

MonaServer provides you with the ability to save data in a **NoSQL** way.

.. contents:: Table of Contents

The *data* global variable
*******************************************

**data** is the name of the global table variable that performs read&write on the disk.

Writing persistent data
===========================================

To write data in the database just assign the table **data**.

.. code-block:: lua

    function onConnection(client, ...)
        data["lastClientId"] = client.id
        
        # save each client connexion time in a table
        if data["clientsConnexions"] == nil then data["clientsConnexions"] = {} end
        data["clientsConnexions"][#data["clientsConnexions"]+1] = os.time()
    end
    
.. note:: If you want to delete some old value (or entire database) you just have to affect *nil* to the corresponding key like this :

.. code-block:: lua

    function clearLastClientId()
        data["lastClientId"] = nil
    end

Reading persistent data
===========================================

To access to a data value it is as simple as this :

.. code-block:: lua

    function onMessage(message)
        NOTE("Last client ID : ", data["lastClientId"])
    end


How does it works?
*******************************************

This system permits fast data retrieving and does not allow data corruption as it use **md5** for integrity checking.

Root application's values (**www**) are stocked in the **data** directory and all sub-applications have their own sub-directory. With this thing in mind you can access to sub-applications's data in each parents applications :

.. code-block:: lua

    function onConnection(client, ...)
    
        if data["subapp"] then INFO("sub data sample : ", data["subapp"]["sample"]) end
    end
    
And respectively you can access to parents's data using the keyword **super** :

.. code-block:: lua

    function onConnection(client, ...)
     
        INFO("parent data : ", super.data["parentValue"])
    end

.. note::

    The entire database is readed at MonaServer's start.
