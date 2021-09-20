function callGamemodeFunction(%gameModeFuncName, %arg0, %arg1, %arg2, %arg3, %arg4, %arg5, %arg6)
{
   if(%data !$= "")
      %data = "\""@%data@"\"";
    
	// has to be a better way to join these into 1 call.
	if(isObject(getScene2D(0)) )
		%activeSceneCount = getScene2DCount();
	else
		%activeSceneCount = getSceneCount();
      
   %hasGameMode = 0;
   for(%i=0; %i < %activeSceneCount; %i++)
   {
	  if(isObject(getScene2D(0)) )
		%gamemodeName = getScene2D(%i).gameModeName;
	  else
		%gamemodeName = getScene(%i).gameModeName;
	
      if(%gamemodeName !$= "")
      {
         //if the scene defines a game mode, go ahead and envoke it here
         if(isObject(%gamemodeName) && %gamemodeName.isMethod(%gameModeFuncName))
         {
            eval(%gamemodeName @ "."@%gameModeFuncName@"(\""@%arg0@"\", \""@%arg1@"\", \""@%arg2@"\", \""@%arg3@"\", \""@%arg4@"\", \""@%arg5@"\", \""@%arg6@"\");" );
            %hasGameMode = 1;
         }
         else
         {
            //if we don't have an object, attempt the static call  
            if(isMethod(%gamemodeName, %gameModeFuncName))
            {
               eval(%gamemodeName @ "::"@%gameModeFuncName@"(\""@%arg0@"\", \""@%arg1@"\", \""@%arg2@"\", \""@%arg3@"\", \""@%arg4@"\", \""@%arg5@"\", \""@%arg6@"\");" );
               %hasGameMode = 1;
            }
         }
      }
   }
   
   //if none of our scenes have gamemodes, we need to kick off a default
   if(%hasGameMode == 0)
   {
      %defaultModeName = ProjectSettings.value("Gameplay/GameModes/defaultModeName");
      if(%defaultModeName !$= "")
      {
         if(isObject(%defaultModeName) && %defaultModeName.isMethod(%gameModeFuncName))
         {
            eval(%defaultModeName @ "."@%gameModeFuncName@"(\""@%arg0@"\", \""@%arg1@"\", \""@%arg2@"\", \""@%arg3@"\", \""@%arg4@"\", \""@%arg5@"\", \""@%arg6@"\");" );
            %hasGameMode = 1;
         }
         else
         {
            if(isMethod(%defaultModeName, %gameModeFuncName))
            {
               eval(%defaultModeName @ "::"@%gameModeFuncName@"(\""@%arg0@"\", \""@%arg1@"\", \""@%arg2@"\", \""@%arg3@"\", \""@%arg4@"\", \""@%arg5@"\", \""@%arg6@"\");" );
               %hasGameMode = 1;
            }  
         }
      }
   }  
   
   return %hasGameMode;
}