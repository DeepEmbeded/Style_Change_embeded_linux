module.exports = {
  "env": {
    "browser": true,
    "es2021": true,
    "node": true
  },
  "extends": [
    "eslint:recommended",         
    'plugin:vue/vue3-recommended', 
    
    'plugin:prettier/recommended',
    'eslint-config-prettier'
  ],
  "parserOptions": {
    "ecmaVersion": 13,
    "sourceType": "module",
    "ecmaFeatures": {
      "modules": true,
      'jsx': true
    },
    "requireConfigFile": false,
    "parser": '@babel/eslint-parser'
  },
  
  'plugins': [
    'vue',
    'prettier' 
  ],
  'globals': {
    defineProps: 'readonly',
    defineEmits: 'readonly',
    defineExpose: 'readonly',
    withDefaults: 'readonly'
  },
  
  "rules": {
    'semi': ['warn', 'never'],           
    'no-console': 'warn',                
    'no-debugger': 'warn',               
    'no-duplicate-case': 'warn',         
    'no-empty': 'warn',                  
    'no-extra-parens': 'warn',           
    'no-func-assign': 'warn',            
    'no-unreachable': 'warn',            
    'no-else-return': 'warn',            
    'no-empty-function': 'warn',         
    'no-lone-blocks': 'warn',            
    'no-multi-spaces': 'warn',           
    'no-redeclare': 'warn',              
    'no-return-assign': 'warn',          
    'no-return-await': 'warn',           
    'no-self-compare': 'warn',           
    'no-useless-catch': 'warn',          
    'no-useless-return': 'warn',         
    'no-mixed-spaces-and-tabs': 'warn',  
    'no-multiple-empty-lines': 'warn',   
    'no-trailing-spaces': 'warn',        
    'no-useless-call': 'warn',           
    'no-var': 'warn',                    
    'no-delete-var': 'off',              
    'no-shadow': 'off',                  
    'dot-notation': 'warn',              
    'default-case': 'warn',              
    'eqeqeq': 'warn',                    
    'curly': 'warn',                     
    'space-before-blocks': 'warn',       
    'space-in-parens': 'warn',           
    'space-infix-ops': 'warn',           
    'space-unary-ops': 'warn',           
    'switch-colon-spacing': 'warn',      
    'arrow-spacing': 'warn',             
    'array-bracket-spacing': 'warn',     
    'brace-style': 'warn',               
    'camelcase': 'warn',                 
    'indent': ['warn', 4],               
    'max-depth': ['warn', 4],            
    'max-statements': ['warn', 100],     
    'max-nested-callbacks': ['warn', 3], 
    'max-statements-per-line': ['warn', { max: 1 }],   
    "quotes": ["warn", "single", "avoid-escape"],      
    "vue/require-default-prop": 0,                     
    "vue/singleline-html-element-content-newline": 0,  
    "vue/multiline-html-element-content-newline": 0,   
    
    'vue/max-attributes-per-line': ['warn', { singleline: 5 }],
    
    "vue/html-indent": ["warn", 4, {
      "attribute": 1,
      "baseIndent": 1,
      "closeBracket": 0,
      "alignAttributesVertically": true,
      "ignores": []
    }],
    
    "vue/html-self-closing": ["error", {
      "html": {
        "void": "always",
        "normal": "never",
        "component": "always"
      },
      "svg": "always",
      "math": "always"
    }]
  }
}