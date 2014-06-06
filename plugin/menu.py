# NtoA
nuke.load('ArnoldRender')

toolbar = nuke.toolbar("Nodes")
toolbar.addCommand( "NtoA/ArnoldRender", "nuke.createNode('ArnoldRender')")
toolbar.addCommand( "NtoA/AiStandard", "nuke.createNode('ntoaStandard')")
toolbar.addCommand( "NtoA/AiShadowCatcher", "nuke.createNode('ntoaShadowCatcher')")
