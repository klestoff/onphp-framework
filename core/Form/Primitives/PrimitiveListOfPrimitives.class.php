<?php
	/**
	 * @author Alexander Klestov <a.klestov@co.wapstart.ru>
	 * @copyright Copyright (c) 2011, Wapstart
	 */
	final class PrimitiveListOfPrimitives extends BasePrimitive
	{
		/**
		 * @var BasePrimitive
		 */
		private $primitive = null;
		
		/**
		 * @return PrimitiveListOfPrimitives
		 */
		public static function create($name) 
		{
			return new self($name);
		}
		
		public function getPrimitive()
		{
			return $this->primitive;
		}
		
		/**
		 * @return PrimitiveListOfPrimitives 
		 */
		public function setPrimitive(BasePrimitive $primitive)
		{
			$this->primitive = $primitive;
			
			return $this;
		}
		
		public function getValue() 
		{
			return $this->getAdoptedValue('getValue');
		}
		
		public function getActualValue() 
		{
			return $this->getAdoptedValue('getActualValue');
		}
		
		public function getSafeValue() 
		{
			return $this->getAdoptedValue('getSafeValue');
		}
		
		public function import($scope) 
		{
			Assert::isNotNull($this->primitive, 'Primitive must be set');
			
			if (!parent::import($scope))
				return null;
			
			if (is_array($this->raw)) {
				$result = true;
				
				$this->value = array();
				
				foreach ($this->raw as $rawValue) {
					$primitive = clone $this->primitive;
					
					$result = $result && $primitive->importValue($rawValue);
					
					$this->value[] = $primitive;
				}
				
				return $result;
			}
			
			return false;
		}
		
		private function getAdoptedValue($method)
		{
			if (!$this->value)
				return null;
			
			$result = array();
			
			foreach ($this->value as $primitive)
				$result[] = $primitive->$method();
			
			return $result;
		}
	}
?>
