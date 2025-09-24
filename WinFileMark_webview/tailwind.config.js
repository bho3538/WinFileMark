module.exports = {
  content: [
    './src/**/*.tsx', 
    './src/**/.component/**/*.tsx',
    './src/**/.hook/**/*.tsx'
  ],
  darkMode: 'class',
  theme: {
    extend: {
      backgroundImage: {
        'locale-en': "url('/src/assets/en.png')",
        'locale-ko': "url('/src/assets/ko.png')",
        'locale-jp': "url('/src/assets/jp.png')",
        'locale-zh': "url('/src/assets/zh.png')",
      },
    },
  },
  plugins: [],
};