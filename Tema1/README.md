  # 1.TASK
  ## 1.1 Descriere
În această temă trebuie să simulați o stație de încărcare pentru un vehicul electric, folosind mai multe LED-uri și butoane. În cadrul acestui task trebuie să țineți cont de stările butonului și să folosiți debouncing, dar și să coordonați toate componentele ca într-un scenariu din viața reală.
  ## 1.2 Detalii tehnice
Led-ul RGB reprezintă disponibilitatea stației. Dacă stația este liberă led-ul va fi verde, iar dacă stația este ocupată se va face roșu.<br />
Led-urile simple reprezintă gradul de încărcare al bateriei, pe care îl vom simula printr-un loader progresiv (L1 = 25%, L2 = 50%, L3 = 75%, L4 = 100%). Loader-ul se încărca prin aprinderea succesivă a led-urilor, la un interval fix de 3s. LED-ul care semnifică procentul curent de încărcare va avea starea de clipire, LED-urile din urma lui fiind aprinse continuu, iar celelalte stinse.<br />
Apăsarea scurtă a butonului de start va porni încărcarea. Apăsarea acestui buton în timpul încărcării nu va face nimic.<br />
Apăsarea lungă a butonului de stop va opri încărcarea forțat și va reseta stația la starea liberă. Apăsarea acestui buton cat timp stația este liberă nu va face nimic.<br />
  ## 1.3 Flow
Starea stației este ‘liberă’. Loader-ul este stins, iar led-ul pentru disponibilitate este verde.<br />
Se apasă butonul pentru start.<br />
Led-ul pentru disponibilitate se face roșu, iar încărcarea începe prin aprinderea primului LED L1.<br />
Led-ul 1 clipește timp de 3s, celelalte fiind stinse.<br />
Dupa încărcarea primului procent de 25% led-ul rămâne aprins și se trece la următorul led, care va începe să clipească.<br />
La finalizarea încărcării toate led-urile vor clipi simultan de 3 ori, iar apoi se vor stinge, pentru a semnaliza finalizarea procesului.<br />
Led-ul pentru disponibilitate se face verde.<br />
Dacă oricând de la pornirea încărcării până la finalizarea acesteia este apăsat lung (min 1s) butonul de stop, încărcarea se întrerupe prin animația de final (toate led-urile clipesc de 3 ori), iar led-ul pentru disponibilitate devine verde.<br />
  # 2.COMPONENTE UTILIZATE
  * 4x LED-uri albastre
  * 1x LED RGB 
  * 2x Butoane 
  * 9x Rezistoare (3x 220, 4x330ohm, 2x 1K)
  * Breadboard
  * Fire
  # 3.POZE
  # 4.VIDEO
  # 5.SCHEMA ELECTRICA
  ![plot](./media/Grand%20Luulia.pdf)
