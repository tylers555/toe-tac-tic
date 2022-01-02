#ifdef SNAIL_JUMPY_DEBUG_BUILD
// TODO(Tyler): This also logs all the data
internal void
DEBUGRenderAllProfileData(layout *Layout){
 for(u32 ProfileIndex = 0;
     ProfileIndex < ProfileData.CurrentBlockIndex+1;
     ProfileIndex++){
  profiled_block *Block = &ProfileData.Blocks[ProfileIndex];
  if(Block->Name == 0){
   continue;
  }
  
  f32 ActualX = Layout->CurrentP.X + (Layout->Advance.X*(f32)Block->Level);
  RenderFormatString(&DebugFont, BLACK, V2(ActualX, Layout->CurrentP.Y), -1.0f,
                     "%s: %'8llucy", Block->Name, Block->CycleCount);
  Layout->CurrentP.Y -= DebugFont.Size;
 }
}

internal void
DEBUGRenderOverlay(){
 layout Layout = MakeLayout(100, OSInput.WindowSize.Height-150, 30, DebugFont.Size, 100, -0.9f);
 if(DebugConfig.Overlay & DebugOverlay_Miscellaneous){
  LayoutString(&Layout, &DebugFont,
               BLACK, "Counter: %.2f", Counter);
  LayoutString(&Layout, &DebugFont,
               BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
  LayoutString(&Layout, &DebugFont,
               BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
  {
   entity_player *Player = EntityManager.Player;
   LayoutString(&Layout, &DebugFont, BLACK, "Player.P: (%f, %f)", 
                Player->P.X, Player->P.Y);
   LayoutString(&Layout, &DebugFont, BLACK, "Player.dP: (%f, %f)", 
                Player->dP.X, Player->dP.Y);
   LayoutString(&Layout, &DebugFont, BLACK, "Player.TargetdP: (%f, %f)", 
                Player->TargetdP.X, Player->TargetdP.Y);
  }
  LayoutString(&Layout, &DebugFont,
               BLACK, "PhysicsDebugger: %u", 
               PhysicsDebugger.Current);
 }
 if(DebugConfig.Overlay & DebugOverlay_Profiler){
  LayoutFps(&Layout);
  DEBUGRenderAllProfileData(&Layout);
 }
}
#else
internal void DEBUGRenderOverlay() {};
#endif