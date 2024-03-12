# HoudiniEngine

This Gem enables the use of [Houdini](https://www.sidefx.com/products/houdini) with [O3DE](http://www.o3de.org).

When this gem is enabled in an O3DE project, a menu item named "Houdini" will appear on O3DE's main menu as well as a toolbar that provides shortcuts to some of the most frequently used features.

# Getting Started

_Warning: These instructions correspond to  a work in progress version of the Houdini integration, many options are not working and may not be in their final configuration_

The first step in using Houdini in O3DE is to open a Houdini Session.

If Houdini is not already running, select "Open Houdini Session Sync" from the Houdini Configuration panel. If Houdini is already running, make sure to enable Houdini Engine, then in O3DE select "Start Session".

To get started using Houdini from O3DE, launch O3DE, there is no need to launch Houdini on its own, we will do this from within the O3DE editor.

Navigate to the Houdini menu and select _Session Controls_

![Session Controls Menu](Docs/Img/image.png)

This will open the following window.

![Session Controls](Docs/Img/image-1.png)

Next, press _Open Houdini Session Sync_

This will launch Houdini and automatically begin a connection.

When O3DE is connected to Houdini we will see the Houdini icon on the bottom left portion of the O3DE viewport

![Houdini is Connected](Docs/Img/image-2.png)

## Sending Geometry to Houdini

From the Houdini menu option select _Node Sync_

![Node Sync Option](Docs/Img/image-4.png)

This will open the following window

Open ![Node Sync](Docs/Img/image-3.png)

Select an entity in O3DE

![alt text](Docs/Img/image-5.png)

The select _Send to Houdini_ from the Node Sync window, this will create an _O3DEContent_ Subnetwork and a _Geometry_ node in Houdini.

![alt text](Docs/Img/image-6.png)

## Working with HDA files

In O3DE we can use the Asset Browser to find HDA files which can be dragged into the O3DE viewport, doing this will create an entity with a HoudiniDigitalAsset component.

![alt text](Docs/Img/image-7.png)

Selecting _OperatorType_ will open a window that will list all Houdini digital assets, select the one that corresponds to the file

![alt text](Docs/Img/image-8.png)

For example for the _he_rock.hda_ file we would select _Sop/GameJam_Rock_.

This is currently a slow procedure, the O3DE editor may seem unresponsive. Once the operation is complete the loaded object will be visible on the Houdini editor, note however, currently the mesh is not visible on O3DE (work in progress).





















*Houdini Engine and the Houdini logo are Trademarks of Side Effects Software Inc. registered in the USA and other countries.*