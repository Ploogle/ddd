# CONSTRUCT 3 - SpriteFont parameters

CharSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,;:?!\"'+-=*%_()[]{}~#&@©®™°^`|/\<>…€$£¢¿¡“”‘’«»‹›„‚·•ÀÁÂÄÃÅĄÆÇĆÐÈÉÊËĘĞÌÍÎÏİŁÑŃÒÓÔÖÕŐØŒŚŞẞÞÙÚÛÜŰÝŸŹŻàáâäãåąæçćðèéêëęğìíîïıłñńòóôöõőøœśşßþùúûüűýÿźżАБВГҐДЕЁЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгґдеёєжзиіїйклмнопрстуфхцчшщъыьэюяČĎĚŇŘŠŤŮŽčďěňřšťůž "

SpacingData = [[5," "],[10,"%@©®…"],[9,"mæœДФЩЮфщю"],[8,"MVWw#&€«»ÆÐŒЖЛМЦШЫджлмцшыď"],[7,"ABDGHKNOPQRSTUXYabdeghknopqsuvxy0123456789?~™$£¢¿ÀÁÂÄÃÅĄĞŁÑŃÒÓÔÖÕŐØŚŞẞÞÙÚÛÜŰÝŸàáâäãåąðèéêëęğñńòóôöõőøśşßþùúûüűýÿАБВГҐЄЗИЙКНОПРТУХЧЪЬЭЯабвгґеёєзийкнопртухчъьэяĎŇŘŠŤŮěňšů"],[6,"CEFLZcfrz\"*_{}^“”„ÇĆÈÉÊËĘŹŻçćłźżЕЁСсČĚŽčřž"],[5,"IJt+-=()[]°/\\<>‹›ÌÍÎÏİîïІЇїť"],[4,"ijl.,;!'`•ìíıі"],[3,":|¡‘’‚·"]]

# End CONSTRUCT 3 - SpriteFont parameters

spaces = {}

for category in SpacingData:
  space, chars = category
  for c in chars:
    spaces[c] = space

print("tracking=0")
for c in CharSet:
  ch = c
  if ch==" ": ch="space"
  print(ch,"\t",spaces[c])