package de.hpi.mocc.jujo.exercise4;

import org.apache.flink.api.common.JobExecutionResult;
import org.apache.flink.api.common.accumulators.DoubleCounter;
import org.apache.flink.api.common.accumulators.IntCounter;
import org.apache.flink.api.common.functions.FlatMapFunction;
import org.apache.flink.api.common.functions.RichFlatMapFunction;
import org.apache.flink.api.common.state.ValueState;
import org.apache.flink.api.common.state.ValueStateDescriptor;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.api.java.utils.ParameterTool;
import org.apache.flink.configuration.Configuration;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.util.Collector;

import java.io.File;
import java.io.PrintWriter;
import java.util.Map;


public class WordCount {

    public static void main(String[] args) throws Exception {

        final ParameterTool params = ParameterTool.fromArgs(args);

        final StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();

        env.getConfig().setGlobalJobParameters(params);

        DataStream<String> text;

        if (!params.has("input")) {
            System.err.println("Use --input to specify valid file input.");
            System.exit(-1);
        } else {
            File inputFile = new File(params.get("input"));
            if (!inputFile.exists() || inputFile.isDirectory()) {
                System.err.println("Provided input file does not exist.");
                System.exit(-1);
            }
        }

        if (!params.has("output")) {
            System.err.println("Use --output to specify file output.");
            System.exit(-1);
        }

        text = env.readTextFile(params.get("input"));

        text.flatMap(new Tokenizer())
            .keyBy(0)
            .flatMap(new Accumulator());

        JobExecutionResult result = env.execute("Streaming WordCount");
        Map<String, Object> resultSet = result.getAllAccumulatorResults();

        PrintWriter pw = new PrintWriter(new File(params.get("output")));
        StringBuilder lineBuilder = new StringBuilder();

        lineBuilder.append("word");
        lineBuilder.append(",");
        lineBuilder.append("occurrences");
        lineBuilder.append("\n");

        for(String word : resultSet.keySet()) {
            lineBuilder.append(word);
            lineBuilder.append(",");
            lineBuilder.append(resultSet.get(word));
            lineBuilder.append("\n");
        }

        pw.write(lineBuilder.toString());
        pw.close();
    }

    public static final class Tokenizer implements FlatMapFunction<String, Tuple2<String, Integer>> {

        @Override
        public void flatMap(String value, Collector<Tuple2<String, Integer>> out) {

            String[] tokens = value.toLowerCase().split("\\W+");

            for (String token : tokens) {
                if (token.length() > 0) {
                    out.collect(new Tuple2<>(token, 1));
                }
            }
        }
    }

    public static class Accumulator extends RichFlatMapFunction<Tuple2<String, Integer>, Tuple2<String, Integer>> {

        ValueState<Boolean> statusAccumulatorExists;

        @Override
        public void open(Configuration config) {
            statusAccumulatorExists = getRuntimeContext().getState(new ValueStateDescriptor<>("statusAccumulatorExists", boolean.class));
        }

        @Override
        public void flatMap(Tuple2<String, Integer> input, Collector<Tuple2<String, Integer>> out) throws Exception {
            if (statusAccumulatorExists.value() == null || !statusAccumulatorExists.value()) {
                getRuntimeContext().addAccumulator(input.f0, new IntCounter());
                statusAccumulatorExists.update(true);
            }

            getRuntimeContext().getAccumulator(input.f0).add(input.f1);
        }
    }
}