ruby run.rb
mv history.js history1.js
ruby run.rb
mv history.js history2.js
ruby run.rb
mv history.js history3.js
ruby merge.rb history1.js history2.js history3.js > merged.js
